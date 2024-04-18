#include <iostream>
#include <random>
#include "thread_pool.hpp"

std::random_device rd;  // 真实随机数产生器

std::mt19937 mt(rd());  // 生成计算随机数

std::uniform_int_distribution<int> dist(-1000, 1000);   // -1000到1000的离散均匀分布

auto rnd = std::bind(dist, mt);

// 设置线程睡眠时间
void simulate_hard_computation() {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
}

// 添加两个数字的简单函数并打印结果
void multiply(const int a, const int b) {
    simulate_hard_computation();
    const int res = a * b;
    std::cout << a << " * " << b << " = " << res << std::endl;
}

// 添加并输出结果
void multiply_output(int& out, const int a, const int b) {
    simulate_hard_computation();
    out = a * b;
    std::cout << a << " * " << b << " = " << out << std::endl;
}

// 结果返回
int multiply_return(const int a, const int b) {
    simulate_hard_computation();
    const int res = a * b;
    std::cout << a << " * " << b << " = " << res << std::endl;
    return res;
}

void example() {
    // 创建有三个线程的线程池
    ThreadPool pool(3);

    // 进程池初始化
    pool.init();

    // 30个乘法操作
    for(int ii = 1; ii <= 3; ++ii) {
        for(int jj = 1; jj <= 10; ++jj) {
            pool.submit(multiply, ii, jj);
        }
    }

    // 使用ref传递的输出参数提交函数
    int output_ref;
    auto future1 = pool.submit(multiply_output, std::ref(output_ref), 5, 6);

    // 等待乘法输出完成
    future1.get();
    std::cout << "上一次操作的结果= " << output_ref << std::endl;

    // 使用return参数提交函数
    auto future2 = pool.submit(multiply_return, 5, 3);

    // 等待乘法输出完成
    int res = future2.get();
    std::cout << "上一次操作的结果= " << res << std::endl;

    // 关闭线程池
    pool.shutdown();

}

int main() {
    example();

    return 0;
}