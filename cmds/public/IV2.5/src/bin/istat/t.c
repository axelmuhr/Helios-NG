typedef int (*PF)(class A*);

class A {
public:
    PF x;

    int f();
};

int A::f () {
    while (x(this)) {
    }
}
