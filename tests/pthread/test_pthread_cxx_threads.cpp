#include <thread>

int main(int argc, char* argv[]) {
  std::thread t([]{});
  t.join();

  std::thread t2([]{});
  t2.join();
  return 0;
}
