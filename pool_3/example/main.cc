#include <iostream>
#include <random>

#include "../include/ThreadPool.h"


std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-1000, 1000);
auto rnd = std::bind(dist, mt);

void simulate_hard_computation() {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
}

// 添加两个数字相乘并打印结果的简单函数
void multiply(const int a, const int b) {
    simulate_hard_computation();
    const int res = a * b;
    std::cout << a << "*" << b << "=" << res << std::endl;
}

// 带输出的相乘
void multiply_output(int & out, const int a, const int b) {
  simulate_hard_computation();
  out = a * b;
  std::cout << a << " * " << b << " = " << out << std::endl;
}

// 带输出的相乘
int multiply_return(const int a, const int b) {
    simulate_hard_computation();
    const int res = a * b;
    std::cout << a << " * " << b << " = " << res << std::endl;
    return res;
}

int main() {
    // 创建一个有三个线程的线程池
    ThreadPool pool(3);

    // 初始化
    pool.init();

    // 提交（部分）九九乘法表
    for (int i = 1; i < 3; ++i) {
        for (int j = 1; j < 10; ++j) {
            pool.submit(multiply, i, j);
        }
    }

    // 提交函数，输出参数由 ref 传递
    int output_ref;
    auto future1 = pool.submit(multiply_output, std::ref(output_ref), 5, 6);

    // 等待乘法输出完成
    future1.get();
    std::cout << "Last operation result is equals to " << output_ref << std::endl;

    // 用return
    auto future2 = pool.submit(multiply_return, 5, 3);
    // 等待乘法输出完成
    int res = future2.get();
    std::cout << "Last operation result is equals to " << res << std::endl;

    pool.shutdown();
    return 0;
}