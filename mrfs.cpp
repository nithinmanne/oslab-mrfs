#include "mrfs.h"

mrfs::mrfs(): FS(nullptr), _key(0), init(false), sb(), key(_key) {}

mrfs::mrfs(const int& key): _key(key), init(true), sb(), key(_key) {
    int shmid = shmget(_key, sizeof sb.size, 0666);
    if(shmid==-1){
        std::cerr << "Shmget Failed for size" << std::endl;
        exit(1);
    }
    auto *sizep = (int*)shmat(shmid, nullptr, 0);
    int size = *sizep;
    shmdt(sizep);
    shmid = shmget(_key, static_cast<size_t>(size), 0666);
    if(shmid==-1){
        std::cerr << "Shmget Failed for FS" << std::endl;
        exit(1);
    }
    FS = shmat(shmid, nullptr, 0);
    sb = FS;
    inode = (block*)FS + sb.block_count;
    curdir = 0;
}

mrfs::mrfs(const mrfs& other): FS(other.FS), _key(other._key), init(other.init), sb(FS), inode(other.inode), curdir(other.curdir), key(_key) {}

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
    auto *sizep = (int*)shmat(shmid, nullptr, 0);
    int size = *sizep;
    shmdt(sizep);
    shmid = shmget(_key, static_cast<size_t>(size), 0666);
    if(shmid==-1){
        std::cerr << "Shmget Failed for size" << std::endl;
        exit(1);
    }
    FS = shmat(shmid, nullptr, 0);
    sb = FS;
    inode = (block*)FS + sb.block_count;
    curdir = 0;
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
    this->inode = other.inode;
    this->curdir = other.curdir;
    sb = this->FS;
    return *this;
}

mrfs::~mrfs(){
    shmdt(FS);
}

mrfs::superblock::superblock(): block_count(0),
                                size(block_count),
                                max_inodes(block_count),
                                used_inodes(block_count),
                                inodes(&block_count),
                                max_blocks(block_count),
                                used_blocks(block_count),
                                blocks(&block_count) {}

mrfs::superblock::superblock(void *FS): size(*(int*)FS),
                                        max_inodes(*(&size+1)),
                                        used_inodes(*(&max_inodes+1)),
                                        inodes(&used_inodes+1),
                                        max_blocks(*(inodes+max_inodes)),
                                        used_blocks(*(&max_blocks+1)),
                                        blocks(&used_blocks+1) {
    block_count = sizeof size + sizeof max_inodes + sizeof used_inodes +
                  max_inodes * sizeof *inodes + sizeof max_blocks + sizeof used_blocks +
                  max_blocks * sizeof *blocks;
    block_count = block_count/sizeof(block) + (block_count%sizeof(block)>0);
}

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
    sb = FS;
    sb.used_inodes = 0;
    for (int i=0;i<sb.max_inodes;i++){
        sb.inodes[i] = 0;
    }
    sb.max_blocks = bsize/sizeof(block);
    sb = FS;
    sb.used_blocks = 0;
    for (int i=0;i<sb.max_blocks;i++){
        sb.blocks[i] = 0;
    }
    for (int i=0;i<sb.block_count;i++){
        sb.blocks[i+sb.used_blocks] = 1;
    }
    sb.used_blocks += sb.block_count;
    inode = (block*)FS + sb.block_count;
    int inode_blocks = sizeof(indexnode)*sb.max_inodes;
    inode_blocks = inode_blocks/sizeof(block) + (inode_blocks%sizeof(block)>0);
    for (int i=0;i<inode_blocks;i++){
        sb.blocks[i+sb.used_blocks] = 1;
    }
    sb.used_blocks += inode_blocks;
    auto rootinode = reqinode();
    auto root = *((indexnode*)inode+rootinode);
    root.filetype = 1;
    root.filesize = sizeof(indexnode);
    root.lastModified = time(nullptr);
    root.lastRead = time(nullptr);
    root.acPermissions = 0666;
    root.direct[0] = reqblock();
    sb.used_blocks++;
    sb.blocks[root.direct[0]] = 1;
    dirlist rootdot = *(dirlist*)((block*)FS + root.direct[0]);
    strcpy(rootdot.name, ".");
    rootdot.inode = rootinode;
    sb.used_inodes++;
    sb.inodes[rootdot.inode] = 1;
    curdir = rootdot.inode;
    return 0;
}

int mrfs::copy_pc2myfs(const char *source, const char *dest) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::copy_myfs2pc(const char *source, const char *dest) const {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::rm_myfs(const char *filename) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::showfile_myfs(const char *filename) const {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::ls_myfs() const {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    auto inodelist = (indexnode*)inode;
    for(int i=0;i<inodelist[curdir].filesize/sizeof(dirlist);i++) {

    }
    return 0;
}

int mrfs::mkdir_myfs(const char *dirname) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::chdir_myfs(const char *dirname) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::rmdir_myfs(const char *dirname) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::open_myfs(const char *filename, char mode) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::close_myfs(int fd) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::read_myfs(int fd, int nbytes, char *buff) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::write_myfs(int fd, int nbytes, char *buff) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::eof_myfs(int fd) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}

int mrfs::dump_myfs (const char *dumpfile) const {
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
    dump_file.write((char*)FS, sb.size);
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
    dump_file.read((char*)&size, sizeof size);
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
    dump_file.read((char*)FS, size);
    if(dump_file.fail()) {
        std::cerr << "Invalid Dump File for size " << size << std::endl;
        return -1;
    }
    init = true;
    sb = FS;
    dump_file.close();
    return 0;
}

int mrfs::status_myfs() const {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    std::cout << "Total Size of File System: " << sb.size << std::endl;
    std::cout << "Occupied Size of File System: " << sb.used_blocks*sizeof(block) << std::endl;
    std::cout << "Free Size of File System: " << sb.size-sb.used_blocks*sizeof(block) << std::endl;
    std::cout << "Total Number of Files: " << sb.used_inodes << std::endl;
    return 0;
}

int mrfs::chmod_myfs(const char *name, int mode) {
    if(!init) {
        std::cerr << "Filesystem Not Initialized" << std::endl;
        return -1;
    }
    return 0;
}




//
