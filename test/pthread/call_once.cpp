#include <iostream>
#include <mutex>
#include <pthread.h>
#include <assert.h>

std::once_flag flag;
int counter = 0;

void do_once() {
    counter++;
}

void* worker_main(void*) {
    std::call_once(flag, do_once);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker_main, NULL);
    pthread_create(&t2, NULL, worker_main, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    std::cout << "Counter: " << counter << std::endl;
    if (counter == 1) {
        std::cout << "Success!" << std::endl;
    } else {
        std::cout << "Failure: counter is " << counter << std::endl;
        return 1;
    }

    return 0;
}
