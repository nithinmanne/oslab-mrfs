#include "mrfs.h"

using namespace std;

int main() {
    mrfs a;
    cout<<0666<<endl;
    cout<<sizeof(char)<<endl;
    a.create_myfs(10);
    a.print();
    a.status_myfs();
    a.dump_myfs("a.dmp");
    cin.get();
    mrfs b;
    b.restore_myfs("a.dmp");
    b.print();
    mrfs c = a;
    c.print();
}
