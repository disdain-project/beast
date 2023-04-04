#include <beast/pipeline.hpp>

// Standard
#include <algorithm>
#include <stdexcept>
#include <thread>

namespace beast {

void Pipeline::addPipe(const std::shared_ptr<Pipe>& pipe) {
  if (pipeIsInPipeline(pipe)) {
    throw std::invalid_argument("Pipe already in this pipeline.");
  }

  std::shared_ptr<ManagedPipe> managed_pipe = std::make_shared<ManagedPipe>();
  managed_pipe->pipe = pipe;
  managed_pipe->should_run = false;
  pipes_.push_back(std::move(managed_pipe));
}

void Pipeline::connectPipes(const std::shared_ptr<Pipe>& source_pipe, uint32_t source_slot_index,
                            const std::shared_ptr<Pipe>& destination_pipe,
                            uint32_t destination_slot_index, uint32_t buffer_size) {
  // Ensure that the pipes are both present already.
  if (!pipeIsInPipeline(source_pipe)) {
    throw std::invalid_argument("Source Pipe not in this Pipeline.");
  }

  if (!pipeIsInPipeline(destination_pipe)) {
    throw std::invalid_argument("Destination Pipe not in this Pipeline.");
  }

  // Ensure this connection doesn't exist yet.
  for (const Connection& connection : connections_) {
    if (connection.source_pipe == source_pipe &&
        connection.source_slot_index == source_slot_index) {
      throw std::invalid_argument("Source port already occupied on Pipe.");
    }

    if (connection.destination_pipe == destination_pipe &&
        connection.destination_slot_index == destination_slot_index) {
      throw std::invalid_argument("Destination port already occupied on Pipe.");
    }
  }

  Connection connection{
      source_pipe, source_slot_index, destination_pipe, destination_slot_index, {}};
  connection.buffer.reserve(buffer_size);
  connections_.push_back(std::move(connection));
}

const std::list<std::shared_ptr<Pipeline::ManagedPipe>>& Pipeline::getPipes() const {
  return pipes_;
}

const std::list<Pipeline::Connection>& Pipeline::getConnections() const { return connections_; }

void Pipeline::start() {
  if (is_running_) {
    throw std::invalid_argument("Pipeline is already running, cannot start it.");
  }

  for (std::shared_ptr<ManagedPipe>& managed_pipe : pipes_) {
    if (!managed_pipe->is_running) {
      managed_pipe->should_run = true;
      std::thread thread(&Pipeline::pipelineWorker, this, std::ref(managed_pipe));
      std::swap(managed_pipe->thread, thread);
      managed_pipe->is_running = true;
    }
  }

  is_running_ = true;
}

void Pipeline::stop() {
  if (!is_running_) {
    throw std::invalid_argument("Pipeline is not running, cannot stop it.");
  }

  for (std::shared_ptr<ManagedPipe>& managed_pipe : pipes_) {
    if (managed_pipe->is_running) {
      managed_pipe->should_run = false;
      managed_pipe->thread.join();
      managed_pipe->is_running = false;
    }
  }

  is_running_ = false;
}

bool Pipeline::isRunning() const { return is_running_; }

bool Pipeline::pipeIsInPipeline(const std::shared_ptr<Pipe>& pipe) const {
  return std::find_if(pipes_.begin(),
                      pipes_.end(),
                      [&pipe](const std::shared_ptr<ManagedPipe>& managed_pipe) {
                        return managed_pipe->pipe == pipe;
                      }) != pipes_.end();
}

void Pipeline::findConnections(std::shared_ptr<ManagedPipe>& managed_pipe,
                               std::vector<Connection*>& source_connections,
                               std::vector<Connection*>& destination_connections) {
  for (Connection& connection : connections_) {
    if (connection.destination_pipe == managed_pipe->pipe) {
      source_connections.push_back(&connection);
    }
    if (connection.source_pipe == managed_pipe->pipe) {
      destination_connections.push_back(&connection);
    }
  }
}

void Pipeline::processOutputSlots(std::shared_ptr<ManagedPipe>& managed_pipe,
                                  const std::vector<Connection*>& destination_connections) {
  for (uint32_t slot_index = 0; slot_index < managed_pipe->pipe->getOutputSlotCount();
       ++slot_index) {
    if (!managed_pipe->pipe->hasOutput(slot_index)) {
      continue;
    }
    auto destination_slot_connection_iter =
        std::find_if(destination_connections.begin(),
                     destination_connections.end(),
                     [slot_index](const Connection* connection) {
                       return connection->destination_slot_index == slot_index;
                     });

    if (destination_slot_connection_iter == destination_connections.end()) {
      continue;
    }

    Connection* destination_slot_connection = *destination_slot_connection_iter;
    while (managed_pipe->pipe->hasOutput(slot_index) &&
           (destination_slot_connection->buffer.size() <
            destination_slot_connection->buffer.capacity())) {
      auto data = managed_pipe->pipe->drawOutput(slot_index);
      destination_slot_connection->buffer.push_back(std::move(data));
    }
  }
}

void Pipeline::processInputSlots(std::shared_ptr<ManagedPipe>& managed_pipe,
                                 const std::vector<Connection*>& source_connections) {
  for (uint32_t slot_index = 0; slot_index < managed_pipe->pipe->getInputSlotCount();
       ++slot_index) {
    auto source_slot_connection_iter =
        std::find_if(source_connections.begin(),
                     source_connections.end(),
                     [slot_index](const Connection* connection) {
                       return connection->source_slot_index == slot_index;
                     });

    if (source_slot_connection_iter == source_connections.end()) {
      continue;
    }

    Connection* source_slot_connection = *source_slot_connection_iter;
    if (source_slot_connection->buffer.empty()) {
      continue;
    }

    while (managed_pipe->pipe->hasSpace() && !source_slot_connection->buffer.empty()) {
      auto data = source_slot_connection->buffer.back();
      source_slot_connection->buffer.pop_back();
      managed_pipe->pipe->addInput(data.data);
    }
  }
}

void Pipeline::pipelineWorker(std::shared_ptr<ManagedPipe>& managed_pipe) {
  std::vector<Connection*> source_connections;
  std::vector<Connection*> destination_connections;
  findConnections(managed_pipe, source_connections, destination_connections);

  while (managed_pipe->should_run) {
    processOutputSlots(managed_pipe, destination_connections);
    processInputSlots(managed_pipe, source_connections);

    const bool outputs_have_enough_space = !managed_pipe->pipe->outputsAreSaturated();
    const bool inputs_have_enough_data = managed_pipe->pipe->inputsAreSaturated();

    if (inputs_have_enough_data && outputs_have_enough_space) {
      managed_pipe->pipe->execute();
    }

    // Limit cycle time.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

} // namespace beast
