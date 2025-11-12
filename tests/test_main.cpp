#include <Poco/Util/Application.h>
#include <Poco/Logger.h>
#include <iostream>
#include <cassert>
#include <functional>
#include <vector>
#include <string>
#include <stdexcept>

// Test framework
class TestRunner {
public:
	static TestRunner& instance() {
		static TestRunner inst;
		return inst;
	}

	void registerTest(const std::string& name, std::function<void()> test) {
		_tests.emplace_back(name, test);
	}

	int runAll() {
		int passed = 0;
		int failed = 0;
		std::cout << "Running " << _tests.size() << " test suites...\n\n";
		for (auto& pair : _tests) {
			std::cout << "[" << pair.first << "] ";
			try {
				pair.second();
				std::cout << "PASSED\n";
				passed++;
			} catch (const std::exception& ex) {
				std::cout << "FAILED: " << ex.what() << "\n";
				failed++;
			}
		}
		std::cout << "\n========================================\n";
		std::cout << "Tests passed: " << passed << "\n";
		std::cout << "Tests failed: " << failed << "\n";
		return failed == 0 ? 0 : 1;
	}

private:
	std::vector<std::pair<std::string, std::function<void()>>> _tests;
};

#define TEST_SUITE(name) \
	void test_##name(); \
	struct TestReg_##name { \
		TestReg_##name() { \
			TestRunner::instance().registerTest(#name, test_##name); \
		} \
	} _testReg_##name; \
	void test_##name()

#define ASSERT(cond, msg) \
	do { \
		if (!(cond)) { \
			throw std::runtime_error(msg); \
		} \
	} while(0)

#define ASSERT_EQ(a, b, msg) \
	ASSERT((a) == (b), msg)

extern void test_crypto();
extern void test_tunnel();
extern void test_auth();
extern void test_integration();

int main(int argc, char** argv) {
	Poco::Util::Application app;
	app.init(argc, argv);
	
	test_crypto();
	test_tunnel();
	test_auth();
	test_integration();
	
	return TestRunner::instance().runAll();
}

