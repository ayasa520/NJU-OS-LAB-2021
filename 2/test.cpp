#include <stdio.h>
class MyClass {
public:
    MyClass();
    MyClass(MyClass &&) = default;
    MyClass(const MyClass &) = default;
    MyClass &operator=(MyClass &&) = default;
    MyClass &operator=(const MyClass &) = default;
    ~MyClass();

private:
};
/**
 *
 *
 */
MyClass::MyClass() {}

MyClass::~MyClass() {}
int main() {
    const char *s = "123\n";
    int len = 4;
    /* _print(s,len,0); */
    printf("\342\240\200");
    /* _print(s,len,1); */
    /* _print(s,len,0); */
    /* _print(s,len,1); */
}
