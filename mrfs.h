#include <iostream>
#include <fstream>
#include <random>
#include <cstdlib>
#include <cstring>
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
    public:
        int block_count;
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
    block *inode;
    class indexnode {
    public:
        int filetype;
        int filesize;
        time_t lastModified;
        time_t lastRead;
        int acPermissions;
        int direct[8];
        int indirect;
        int doubleindirect;
    };
    inline int reqblock() {
        for(int i=0;i<sb.max_blocks;i++)
            if(sb.blocks[i]==0) return i;
        return -1;
    }
    inline int16_t reqinode() {
        for(int16_t i=0;i<sb.max_inodes;i++)
            if(sb.inodes[i]==0) return i;
        return -1;
    }
    class dirlist {
    public:
        char name[30];
        int16_t inode;
    };
    int curdir;

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
        cout<<"Super Blocks: "<<sb.block_count<<endl;
        cout<<"Size of Inode: "<<sizeof(indexnode)<<endl;
        cout<<"Size of Directory Listing: "<<sizeof(dirlist)<<endl;
    }

    mrfs();
    mrfs(const int& key);   //NOLINT
    mrfs(const mrfs& other);
    mrfs& operator=(const int& key);
    mrfs& operator=(const mrfs& other);
    ~mrfs();

    int create_myfs(int size);
    int copy_pc2myfs(const char *source, const char *dest);
    int copy_myfs2pc(const char *source, const char *dest) const;
    int rm_myfs(const char *filename);
    int showfile_myfs(const char *filename) const;
    int ls_myfs() const;
    int mkdir_myfs(const char *dirname);
    int chdir_myfs(const char *dirname);
    int rmdir_myfs(const char *dirname);
    int open_myfs(const char *filename, char mode);
    int close_myfs(int fd);
    int read_myfs(int fd, int nbytes, char *buff);
    int write_myfs(int fd, int nbytes, char *buff);
    int eof_myfs(int fd);
    int dump_myfs (const char *dumpfile) const;
    int restore_myfs (const char *dumpfile);
    int status_myfs() const;
    int chmod_myfs(const char *name, int mode);

};
