#include <catch2/catch.hpp>

#include <beast/beast.hpp>

class MockPipe : public beast::EvolutionPipe {
 public:
  explicit MockPipe(uint32_t max_candidates) : beast::EvolutionPipe(max_candidates) {}

  [[nodiscard]] double evaluate(const std::vector<unsigned char>& /*program_data*/) override {
    evaluate_call_count_++;
    return 1.0;
  }

  [[nodiscard]] uint32_t getEvaluateCallCount() const { return evaluate_call_count_; }

 private:
  uint32_t evaluate_call_count_ = 0;
};

TEST_CASE("pipe_calls_evaluate_on_evolve", "pipe") {
  const std::vector<unsigned char> candidate = {};
  const int32_t max_population = 10;

  MockPipe pipe(max_population);
  for (uint32_t idx = 0; idx < max_population; ++idx) {
    pipe.addInput(0, candidate);
  }

  pipe.execute();

  REQUIRE(pipe.getEvaluateCallCount() > 0);
}
