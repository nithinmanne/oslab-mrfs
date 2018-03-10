#include "mrfs.h"

mrfs::mrfs(): FS(nullptr), _key(0), init(false), sb(), key(_key) {}

mrfs::mrfs(const int& key): _key(key), init(true), sb(FS), key(_key) {
    int shmid = shmget(_key, sizeof sb.size, 0666);
    if(shmid==-1){
        std::cerr << "Shmget Failed for size" << std::endl;
        exit(1);
    }
    int size = *(static_cast<int*>(shmat(shmid, nullptr, 0)));
    shmid = shmget(_key, static_cast<size_t>(size), 0666);
    if(shmid==-1){
        std::cerr << "Shmget Failed for FS" << std::endl;
        exit(1);
    }
    FS = shmat(shmid, nullptr, 0);
    sb = FS;
}

mrfs::mrfs(const mrfs& other): FS(other.FS), _key(other._key), init(other.init), sb(FS), key(_key) {}

mrfs& mrfs::operator=(const int& key) {
    if(init) {
      std::cerr << "Filesystem Already Initialized" << std::endl;
      return *this;
    }
    _key = key;
    init = true;
    int shmid = shmget(_key, sizeof sb.size, 0666);
    if(shmid==-1){
        std::cerr << "Shmget Failed for size" << std::endl;
        exit(1);
    }
    int size = *(static_cast<int*>(shmat(shmid, nullptr, 0)));
    shmid = shmget(_key, static_cast<size_t>(size), 0666);
    if(shmid==-1){
        std::cerr << "Shmget Failed for size" << std::endl;
        exit(1);
    }
    FS = shmat(shmid, nullptr, 0);
    sb = FS;
    return *this;
}



mrfs& mrfs::operator=(const mrfs& other) {
    if(init) {
      std::cerr << "Filesystem Already Initialized" << std::endl;
      return *this;
    }
    this->FS = other.FS;
    this->_key = other._key;
    this->init = other.init;
    return *this;
}

mrfs::~mrfs(){
    shmdt(FS);
}

mrfs::superblock::superblock(): temp(0),
                                size(temp),
                                max_inodes(temp),
                                used_inodes(temp),
                                inodes(&temp),
                                max_blocks(temp),
                                used_blocks(temp),
                                blocks(&temp) {}

mrfs::superblock::superblock(void *FS): temp(0),
                                        size(*static_cast<int*>(FS)),
                                        max_inodes(*(&size+1)),
                                        used_inodes(*(&max_inodes+1)),
                                        inodes(&used_inodes+1),
                                        max_blocks(*(inodes+max_inodes)),
                                        used_blocks(*(&max_blocks+1)),
                                        blocks(&used_blocks+1) {}

mrfs::superblock& mrfs::superblock::operator=(void* FS) {
    new (this) superblock(FS);
    return *this;
}

int mrfs::create_myfs(int size){
    if(init){
        std::cerr << "Filesystem Already Initialized" << std::endl;
        return -1;
    }
    std::minstd_rand rand;
    rand.seed(static_cast<unsigned long>(time(nullptr)));
    _key = static_cast<int>(rand());
    int bsize = size*1024*1024;
    int shmid = shmget(_key, static_cast<size_t>(bsize), IPC_CREAT | 0666);
    if(shmid==-1){
        std::cerr << "Shmget Failed for FS" << std::endl;
        return -1;
    }
    FS = shmat(shmid, nullptr, 0);
    init = true;
    sb = FS;
    sb.size = bsize;
    sb.max_inodes = 1000;
    sb.used_inodes = 0;
    for (int i=0;i<sb.max_inodes;i++){
        sb.inodes[i] = 0;
    }
    sb.max_blocks = bsize/sizeof(block);
    for (int i=0;i<sb.max_blocks;i++){
        sb.blocks[i] = 0;
    }
    return 0;
}

int mrfs::dump_myfs (const char *dumpfile) {
    if(!init) {
      std::cerr << "Filesystem Not Initialized" << std::endl;
      return -1;
    }
    std::ofstream dump_file;
    dump_file.open(dumpfile, std::ios::out|std::ios::binary);
    if(dump_file.fail()) {
        std::cerr << "File Open Failed" << std::endl;
        return -1;
    }
    dump_file.write(static_cast<char*>(FS), sb.size);
    dump_file.close();
    return 0;
}

int mrfs::restore_myfs (const char *dumpfile) {
    if(init) {
      std::cerr << "Filesystem Already Initialized" << std::endl;
      return -1;
    }
    std::ifstream dump_file;
    dump_file.open(dumpfile, std::ios::in|std::ios::binary);
    if(dump_file.fail()) {
        std::cerr << "File Open Failed" << std::endl;
        return -1;
    }
    int size;
    dump_file.read(static_cast<char*>(&size), sizeof size);
    std::minstd_rand rand;
    rand.seed(static_cast<unsigned long>(time(nullptr)));
    _key = static_cast<int>(rand());
    int shmid = shmget(_key, static_cast<size_t>(size), IPC_CREAT | 0666);
    if(shmid==-1){
        std::cerr << "Shmget Failed for FS" << std::endl;
        return -1;
    }
    FS = shmat(shmid, nullptr, 0);
    dump_file.seekg(0, std::ios::beg);
    dump_file.read(static_cast<char*>(FS), sb.size);
    if(dump_file.fail()||!dump_file.eof()) {
        std::cerr << "Invalid Dump File" << std::endl;
        return -1;
    }
    init = true;
    sb = FS;
    dump_file.close();
    return 0;
}




//
