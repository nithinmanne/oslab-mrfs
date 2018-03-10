#include <iostream>
#include <fstream>
#include <random>
#include <cstdlib>
#include <ctime>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

class mrfs {
private:
    void *FS;
    int _key;
    bool init;
    class superblock {
    private:
        int temp;
    public:
        int& size;
        int& max_inodes;
        int& used_inodes;
        int* inodes;
        int& max_blocks;
        int& used_blocks;
        int* blocks;
        superblock();
        explicit superblock(void *FS);
        superblock& operator=(void* FS);
    };
    superblock sb;
    class block {
        char data[256];
    };

public:
    const int &key;
    inline void print() {
        using std::cout;
        using std::endl;
        cout<<"Key: "<<key<<endl;
        cout<<"Size: "<<sb.size<<endl;
        cout<<"Max Inodes: "<<sb.max_inodes<<endl;
        cout<<"Used Inodes: "<<sb.used_inodes<<endl;
        cout<<"Max Blocks: "<<sb.max_blocks<<endl;
        cout<<"Used Blocks: "<<sb.used_blocks<<endl;
    }

    mrfs();
    explicit mrfs(const int& key);
    mrfs(const mrfs& other);
    mrfs& operator=(const int& key);
    mrfs& operator=(const mrfs& other);
    ~mrfs();

    int create_myfs(int size);
    int dump_myfs (const char *dumpfile);
    int restore_myfs (const char *dumpfile);


};
