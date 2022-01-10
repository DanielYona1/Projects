#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

void decToBinary(int n, char &c)
{
    c = '\0';
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}

// #define SYS_CALL
// ============================================================================
class fsInode
{
    int fileSize;
    int block_in_use;
    int *directBlocks;
    int singleInDirect;
    int num_of_direct_blocks;
    int block_size;
    int num_of_blocks; // number of total blocks for one file to write in
    int last_block; // number of the last block was used
    int indirectOffset; // indirect block offset

public:
    fsInode(int _block_size, int _num_of_direct_blocks)
    {
        fileSize = 0;
        block_in_use = 0;
        num_of_blocks = _num_of_direct_blocks + _block_size;
        block_size = _block_size;
        last_block = -1;
        indirectOffset = 0;
        num_of_direct_blocks = _num_of_direct_blocks;
        directBlocks = new int[num_of_direct_blocks];
        assert(directBlocks);
        for (int i = 0; i < num_of_direct_blocks; i++)
        {
            directBlocks[i] = -1;
        }
        singleInDirect = -1;
    }

    ~fsInode()
    {
        delete directBlocks;
    }

    int getFileSize()
    {
        return this->fileSize;
    }

    int getNumOfBlocks()
    {
        return this->num_of_direct_blocks + this->block_size;
    }

    int getBlockInUse()
    {
        return this->block_in_use;
    }
    int getNumOfDirectBlocks()
    {
        return this->num_of_direct_blocks;
    }

    int *getDirectBlock()
    {
        return this->directBlocks;
    }

    int getLastBlock()
    {
        return this->last_block;
    }
    void setLastBlock(int num)
    {
        this->last_block = num;
    }

    void plusBlockInUse()
    {
        this->block_in_use++;
    }
    void setFileSize(int num)
    {
        this->fileSize += num;
    }
    int getSingleIndirect()
    {
        return this->singleInDirect;
    }
    void setSingleIndirect(int num)
    {
        this->singleInDirect = num;
    }
    int getIndirectOffset()
    {
        return indirectOffset;
    }
    void plusIndirectOffset()
    {
        indirectOffset++;
    }
};

// ============================================================================
class FileDescriptor
{
    pair<string, fsInode *> file;
    bool inUse;
    bool is_fd_deleted; // to mark if fd was deleted

public:
    FileDescriptor(string FileName, fsInode *fsi)
    {
        file.first = FileName;
        file.second = fsi;
        is_fd_deleted = false;
        inUse = true;
    }

    string getFileName()
    {
        return file.first;
    }

    void setFileName(string name)
    {
        file.first = name;
    }

    fsInode *getInode()
    {

        return file.second;
    }

    void setInode(fsInode* node)
    {
        file.second = node;
    }

    bool isInUse()
    {
        return (inUse);
    }
    void setInUse(bool _inUse)
    {
        inUse = _inUse;
    }
    bool getIsFdDeleted()
    {
        return is_fd_deleted;
    }
    void setIsFdDeleted(bool _isDeleted)
    {
        is_fd_deleted = _isDeleted;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk
{
    FILE *sim_disk_fd;

    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode *> MainDir;

    // OpenFileDescriptors --  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    vector<FileDescriptor> OpenFileDescriptors;

    int direct_enteris;
    int block_size;

public:
    // ------------------------------------------------------------------------
    fsDisk()
    {
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        assert(sim_disk_fd);
        for (int i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
    }

    ~fsDisk()
    {
        free(BitVector);
        free(sim_disk_fd);
        OpenFileDescriptors.~vector();
        MainDir.~map();
    }

    // ------------------------------------------------------------------------
    void listAll()
    {
        int i = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {
            cout << "index: " << i << ": FileName: " << it->getFileName() << " , isInUse: " << it->isInUse() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
        }
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    void fsFormat(int blockSize = 4, int direct_Enteris_ = 3)
    {

        block_size = blockSize;
        direct_enteris = direct_Enteris_;
        for (int i = 0; i < OpenFileDescriptors.size(); i++) // free the memory of the file OpenFileDescripto
        {
            OpenFileDescriptors[i].~FileDescriptor();
        }
        OpenFileDescriptors.clear();
        BitVectorSize = DISK_SIZE / blockSize;
        BitVector = (int *)malloc(BitVectorSize * sizeof(int));
        assert(BitVector);
        for (int i = 0; i < BitVectorSize; i++)
        {
            BitVector[i] = 0;
        }
        is_formated = true;
        cout << "Format Disk: number of blocks:  " << DISK_SIZE / block_size << endl;
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName)
    {
        if (is_formated == false) // check if the disk was formated
        {
            return -1;
        }
        for(int i = 0 ; i < OpenFileDescriptors.size() ; i++) // check if there is not a file with the same name already
        {
            if(OpenFileDescriptors[i].getFileName().compare(fileName) == 0 && OpenFileDescriptors[i].getIsFdDeleted() == false )
            {
                cout << "This file name is already exist" << endl;
                return -1;
            }
        }
        fsInode *inode = new fsInode(this->block_size, this->direct_enteris); // create a node
        FileDescriptor file_des = FileDescriptor(fileName, inode); // cread a fd
        pair<string, fsInode *> p; // create a pair
        p.first = fileName;
        p.second = file_des.getInode();
        MainDir.insert(p); // insert the pair to the map
        for(int i = 0 ; i < OpenFileDescriptors.size() ; i++) // checkif there is fd clear that was deleted and puts the new file in there
        {
            if(OpenFileDescriptors[i].getIsFdDeleted() == true)
            {
            this->OpenFileDescriptors[i].setInode(inode);
            this->OpenFileDescriptors[i].setFileName(fileName);
            this->OpenFileDescriptors[i].setInUse(true); // open the file
            this->OpenFileDescriptors[i].setIsFdDeleted(false); // return the fd to be not deleted
            return i;
            }
        }
        this->OpenFileDescriptors.push_back(file_des); // in case there is not fd was deleted put the new fd in the back of the vector 
        return (OpenFileDescriptors.size() - 1);
    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName)
    {
        for (int i = 0; i < OpenFileDescriptors.size(); i++)
        {
            if (OpenFileDescriptors[i].getFileName().compare(fileName) == 0) // check what is the fd with the same name
            {
                if(OpenFileDescriptors[i].getIsFdDeleted() == true) // check if the fd was not deleted
                {
                    cout << "file was deleted" << endl;
                    return -1;
                }
                else if (OpenFileDescriptors[i].isInUse() == false) // check if the file is close
                {
                    OpenFileDescriptors[i].setInUse(true);
                    return i;
                }
                else if(OpenFileDescriptors[i].isInUse() == true) // check if the file is not open already
                {
                    cout << "file is already open" << endl;
                    return -1;
                }
            }
        }
        return -1;
    }
    // ------------------------------------------------------------------------
    string CloseFile(int fd)
    {
        if (fd >= OpenFileDescriptors.size()) // check if there a file with this fd
        {
            cout << "no such file" << endl;
            return "-1";
        }
        if(OpenFileDescriptors.at(fd).getIsFdDeleted() == true) // check if the file was not deleted
        {
            cout << "file was deleted" << endl;
            return "-1";
        }
        if (OpenFileDescriptors.at(fd).isInUse() == true) // check if the file is open
        {
            OpenFileDescriptors.at(fd).setInUse(false); // close the file
            return OpenFileDescriptors.at(fd).getFileName();
        }
        else if(OpenFileDescriptors.at(fd).isInUse() == false) // check if the file is already close
        {
            cout << "file is already close" << endl;
            return "-1";
        }
    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len)
    {
        if(fd >= OpenFileDescriptors.size()) // check if there is a file with this fd
        {
            cout << "no such file" << endl;
            return -1;
        }
        fsInode *node = OpenFileDescriptors.at(fd).getInode();
        int offset = node->getFileSize() % block_size; // offset inside the last block written to
        int space = 0; // how much free space in the last block was used
        int last = node->getLastBlock();
        if (last != -1) // if the the file was written before
        {
            space = (block_size - offset) % block_size; // space is the free space in this block
        }
        bool is_open = OpenFileDescriptors.at(fd).isInUse();
        if (node->getFileSize() + len > (direct_enteris + block_size) * block_size) // check if there is enogh space in the file
        {
            cout << "not enough space" << endl;
            return -1;
        }
        if (is_formated == false) // check if the disk was formated
        {
            cout << "not formated" << endl;
            return -1;
        }
        if(OpenFileDescriptors[fd].getIsFdDeleted() == true) // check if the file was deleted
        {
            cout << "file was deleted" << endl;
            return -1;
        }
        if (is_open == false) // check if the file is open
        {
            cout << "file is not open" << endl;
            return -1;
        }
        if (fd >= OpenFileDescriptors.size()) // check if there is a file with this fd
        {
            cout << "no such file" << endl;
            return -1;
        }
        int clear = 0;
        for (int i = 0; i < BitVectorSize; i++) // check how much clear blocks there are 
        {
            if (BitVector[i] == 0)
            {
                clear++;
            }
        }
        if ((clear * block_size) + space < len) // check if there is enough space in the disk
        {
            cout << "not enough place in the disk" << endl;
            return -1;
        }
        if (space > 0 && node->getLastBlock() != -1) // check if there is a free space in the last block was used
        {
            lseek(sim_disk_fd->_fileno, (node->getLastBlock() * block_size) + offset, SEEK_SET); // go to the last block to the free space inside the block
            if(write(sim_disk_fd->_fileno, buf, space) == -1) // write to the block the chars 
                perror("can not write");
            buf += space; // move the pointer after the chars was written
        }
        int spare = len - space; // how many chars left to write out of buf
        while (spare > 0)
        {
            for (int i = 0; i < BitVectorSize; i++)
            {
                if (BitVector[i] == 0) // check the next free block
                {
                    if (node->getDirectBlock()[direct_enteris - 1] == -1) // if there is at least one direct block that available
                    {
                        lseek(sim_disk_fd->_fileno, i * block_size, SEEK_SET); // go to the free block place
                        if (spare > block_size) // if there are more than "block_size" chars to write
                        {
                            if(write(sim_disk_fd->_fileno, buf, block_size) == -1) // write to the disk 
                                perror("can not write");
                        }
                        else // if there is less than "block_size" chars to write
                        {
                            if(write(sim_disk_fd->_fileno, buf, spare) == -1) // write to the disk
                                perror("can not write");
                        }
                        node->setLastBlock(i); // update the last block was used 
                        BitVector[i] = 1; // mark this block as unavailable
                        node->plusBlockInUse(); // update the number of blocks used 
                        buf += block_size; // move the pointer to the next "block size" chars to write
                        spare = spare - block_size; // update the number of chars left to write
                        for (int j = 0; j < direct_enteris; j++) // go through all the direct block indexes
                        {
                            if (node->getDirectBlock()[j] == -1) // if the direct block is not used
                            {
                                node->getDirectBlock()[j] = i; // put the block we just used in the direct block 
                                break;
                            }
                        }
                        if (spare <= 0) // if there is not chars left to write
                        {
                            break;
                        }
                        continue;
                    }
                    if (node->getDirectBlock()[direct_enteris - 1] != -1 && node->getSingleIndirect() == -1) // if there is not an other direct block to use and single indirect block was not define yet
                    {
                        node->setSingleIndirect(i); // set the next free block as single indirect block
                        BitVector[i] = 1; // update this block to be used 
                        continue;
                    }
                    if (node->getSingleIndirect() != -1) // if the single indirect block was define already
                    {
                        lseek(sim_disk_fd->_fileno, node->getSingleIndirect() * block_size + node->getIndirectOffset(), SEEK_SET); // go to the write place in the single indirect block
                        node->plusIndirectOffset(); // update the single indirect block offset
                        char c;
                        char* c_ptr = &c;
                        decToBinary(i, c);
                        if(write(sim_disk_fd->_fileno, c_ptr, sizeof(char)) == -1) // write the number of the available block in binary in the single indirect block
                            perror("can not write");
                        lseek(sim_disk_fd->_fileno, i * block_size, SEEK_SET); // go to the avaialable block place in the disk
                        if (spare > block_size) // if there are more than "block size" chars to write
                        {
                            if(write(sim_disk_fd->_fileno, buf, block_size) == -1) // write the chars in the disk
                                perror("can not write");
                        }
                        else // if there are "block_size" chars or less to write 
                        {
                            if(write(sim_disk_fd->_fileno, buf, spare) == -1) // write into the disk
                                perror("can not write");
                        }
                        node->setLastBlock(i); // update the last block was used
                        BitVector[i] = 1; // update that the block is unavailable
                        node->plusBlockInUse(); // update the number of blocks used
                        buf += block_size; // move the pionter to the next "block size" chars to write
                        spare = spare - block_size; // update the number of chars left to write 
                        if (spare <= 0) // if there is nothing left to write 
                        {
                            break;
                        }
                    }
                }
            }
        }
        node->setFileSize(len); // update the file size
    }

    // ------------------------------------------------------------------------
    int DelFile(string FileName)
    {
        int index_block = 0;
        for(int i = 0 ; i < OpenFileDescriptors.size() ; i++){ // go through all the used file descriptor
            if(OpenFileDescriptors[i].getFileName().compare(FileName) == 0) // if the name is the same
            {
                fsInode* node = OpenFileDescriptors[i].getInode();
                for(int i = 0 ; i < direct_enteris ; i++) // go through all the direct blocks
                {
                    index_block = node->getDirectBlock()[i]; //  the number of the direct block
                    BitVector[index_block] = 0; // update the direct block to be unused

                }
                if(node->getSingleIndirect() != -1) // if single indirect block was define
                {
                    for(int i = 0 ; i < node->getIndirectOffset() ; i++) // go through all the blocks was written in the single indirect block
                    {
                        char* one = (char*)malloc(sizeof(char));
                        assert(one);
                        lseek(sim_disk_fd->_fileno , node->getSingleIndirect()*block_size + i , SEEK_SET); // go to the single indirect block in the disk with the right offset
                        if(read(sim_disk_fd->_fileno , one , sizeof(char)) == -1) // read the number of the block in binary
                            perror("can not read");
                        int index = one[0]; // the number of the block from binary to int
                        BitVector[index] = 0; // update the block as unavailable
                    }
                }
                    for(auto i = begin (MainDir); i != end(MainDir) ; ++i) // go through the mainDir
                    {
                        if(i->first.compare(FileName)) // if the name is the same
                        {
                            MainDir.erase(i); // remove the file from the mainDir
                            break;
                        }
                    }
                    BitVector[node->getSingleIndirect()]= 0 ; // update the single indirect block as unavailable
                    OpenFileDescriptors[i].getInode()->~fsInode(); // remove the Inode
                    OpenFileDescriptors[i].setInUse(false); // close the file
                    OpenFileDescriptors[i].setIsFdDeleted(true); // mark this fd as deleted 
                    return i;
                    break;
                }
                if(i == OpenFileDescriptors.size())
                    cout << "no such file" << endl;
        }   
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len)
    {

        fsInode *node = OpenFileDescriptors[fd].getInode();
        if (node->getFileSize() < len) // if the lenth of the chars to read bigger than the file size read all the file 
        {
            len = node->getFileSize();
        }
        if (OpenFileDescriptors.size() < fd) // if there is not a file with this fd
        {
            cout << "no such file" << endl;
            return -1;
        }
        buf[0] = '\0'; // update the buf to be empty
        buf[len]; // update the buf lenth
        int spare = len; // how many chars left to read 
        char * copy;
        int single_indirect_offset = 0; 
        for (int i = 0; i < node->getNumOfBlocks(); i++) // go through the max blocks can be used
        {
            if (spare >= block_size) // if there are more than "block_size" chars left to read
            {
                if (i < direct_enteris) // read the direct blocks first
                {
                    copy = (char*)malloc(block_size);
                    assert(copy);
                    int index = node->getDirectBlock()[i]; // get the number of the direct blocks
                    lseek(sim_disk_fd->_fileno, index * block_size, SEEK_SET); // go to the block place in the disk
                    if(read(sim_disk_fd->_fileno, copy, block_size) == -1) // read the chars from the disk into copy
                        perror("can not read");
                    strcat(buf,copy); // append copy to buf
                    spare -= block_size; // update the chars left to read
                }
                else if (i >= direct_enteris) // after we read all the direct blocks 
                {
                    copy = (char*)malloc(block_size);
                    assert(copy);
                    char* one = (char*)malloc(sizeof(char));
                    assert(one);
                    int single_indirect_index = node->getSingleIndirect(); // get the number of single indirect block
                    lseek(sim_disk_fd->_fileno , single_indirect_index*block_size + single_indirect_offset , SEEK_SET); // go the indirect block in the disk with the right offset
                    single_indirect_offset++;
                    if(read(sim_disk_fd->_fileno , one , sizeof(char)) == -1) // read the number of the block in the single indirect block into one  
                        perror("can not read");
                    int index = (int)one[0]; // the number of the block from binary to int
                    lseek(sim_disk_fd->_fileno, index * block_size, SEEK_SET); // go to the block in the disk
                    if(read(sim_disk_fd->_fileno, copy, block_size) == -1) // read from the disk into copy "block size" chars
                        perror("can not read");
                    strcat(buf,copy); // appened  copy to buf
                    spare -= block_size; // update the number of chars left to read
                }
            }
            else if (spare < block_size) // if there is less than "block size" chars to read
            {
                if (i < direct_enteris) // read from the direct block first
                {
                    copy = (char*)malloc(spare);
                    assert(copy);
                    int index = node->getDirectBlock()[i]; // get the number of the direct blocks
                    lseek(sim_disk_fd->_fileno, index * block_size, SEEK_SET);// go to the block place in the disk
                    if(read(sim_disk_fd->_fileno, copy, spare) == -1) // read what left to read into copy 
                        perror("can not read");
                    strcat(buf,copy); // appened copy into buf
                    break; // there is nothing left to read
                }
                else if (i >= direct_enteris) // after we read all the direct blocks
                {
                    char* one = (char*)malloc(sizeof(char));
                    copy = (char*)malloc(spare);
                    assert(copy);
                    int single_indirect_index = node->getSingleIndirect(); // get the number of the single indirect block
                    lseek(sim_disk_fd->_fileno , single_indirect_index*block_size + single_indirect_offset , SEEK_SET); // go to the single indirect block place in the disk with the right offset
                    single_indirect_offset++;
                    if(read(sim_disk_fd->_fileno , one , sizeof(char)) == -1) // read the number of the block in the single indrect block in binary
                        perror("can not read");
                    int index = (int)one[0]; // the number of the block from binary to int 
                    lseek(sim_disk_fd->_fileno, index * block_size, SEEK_SET); // go to block place in the disk
                    if(read(sim_disk_fd->_fileno, copy, spare) == 1) // read from the block what left to read into copy
                        perror("can not read");
                    strcat(buf,copy); // appened copy into buf
                    break; // there is nothing left to read 
                }
            }
        }
    }
};

int main()
{
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while (1)
    {
        cin >> cmd_;
        switch (cmd_)
        {
        case 0: // exit
            delete fs;
            exit(0);
            break;

        case 1: // list-file
            fs->listAll();
            break;

        case 2: // format
            cin >> blockSize;
            cin >> direct_entries;
            fs->fsFormat(blockSize, direct_entries);
            break;

        case 3: // creat-file
            cin >> fileName;
            _fd = fs->CreateFile(fileName);
            cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 4: // open-file
            cin >> fileName;
            _fd = fs->OpenFile(fileName);
            cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 5: // close-file
            cin >> _fd;
            fileName = fs->CloseFile(_fd);
            cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 6: // write-file
            cin >> _fd;
            cin >> str_to_write;
            fs->WriteToFile(_fd, str_to_write, strlen(str_to_write));
            break;

        case 7: // read-file
            cin >> _fd;
            cin >> size_to_read;
            fs->ReadFromFile(_fd, str_to_read, size_to_read);
            cout << "ReadFromFile: " << str_to_read << endl;
            break;

        case 8: // delete file
            cin >> fileName;
            _fd = fs->DelFile(fileName);
            cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;
            default:
            break;
        }
    }
}