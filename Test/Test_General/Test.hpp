#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <cstring>


class TestFactory {
public:
	TestFactory() = default;

	static TestFactory* GetInstance() {
		static TestFactory instance;
		return &instance;
	}

	std::vector<std::string> GetTests() {
		std::vector<std::string> res;
		for (auto& v : testMap) {
			res.push_back(v.first);
		}
		return res;
	}
	int Run(std::string test) {
		auto it = testMap.find(test);
		if (it != testMap.end()) {
			return it->second();
		}
		else {
			return -1;
		}
	}

	template <class TestT>
	void RegisterTest() {
		testMap.insert({ TestT().Name(), []() -> int {TestT t; return t.Run(); } });
	}
private:
	struct StrCmp {
		bool operator()(const std::string& lhs, const std::string& rhs) const {
			return strcmp(lhs.c_str(), rhs.c_str()) < 0;
		}
	};
	std::map<std::string, std::function<int()>, TestFactory::StrCmp> testMap;
};


template <class T>
class AutoRegisterTest {
public:
	virtual ~AutoRegisterTest() = default;
	virtual std::string Name() const = 0;
	virtual int Run() { return helper.a; }
private:
	struct Helper {
		Helper() {
			TestFactory::GetInstance()->RegisterTest<T>();
			//std::cout << "Registering" << std::endl;
			a = -1;
		}
		int a;
	};
	static Helper helper;
};

template <class T>
typename AutoRegisterTest<T>::Helper AutoRegisterTest<T>::helper;