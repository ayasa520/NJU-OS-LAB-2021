#include <stdarg.h>
#include <regex>
#include <string>
#include <vector>
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
struct RootEntry {
    char DIR_Name[11];   //文件名 8 字节, 扩展名 3 个字节
    uchar DIR_Attr;      // 文件属性, 16 文件夹 32 文件 0 空
    uchar reserve[10];   // 保留位
    ushort DIR_WrtTime;  // 最后一次写入时间
    ushort DIR_WrtDate;  // 最后一次写入日期
    ushort DIR_FstClus;  // 文件开始的簇号
    uint DIR_FileSize;   // 文件大小
};
extern "C" {
void _print(const char *s, int len);
}

const char *INVALID_OPTION = "invalid option\n";
const char *RED = "\033[31m";
const char *DEFAULT = "\033[m";
const char *FILE_NOT_FOUND = "no such file or directory\n";
const char *PARSE_ERROR = "parse error\n";
const char *COMMAND_NOT_FOUND = "command not found\n";
const char* NOT_A_FILE = "not a file\n";
struct Fat12Header {
    char BS_OEMName[8];  // OEM字符串，必须为8个字符，不足以空格填空
    ushort BPB_BytsPerSec;  // 每扇区字节数
    uchar BPB_SecPerClus;   // 每簇占用的扇区数
    ushort BPB_RsvdSecCnt;  // Boot占用的扇区数
    uchar BPB_NumFATs;      // FAT表的记录数
    ushort BPB_RootEntCnt;  // 最大根目录文件数
    ushort BPB_TotSec16;    // 每个FAT占用扇区数
    uchar BPB_Media;        // 媒体描述符
    ushort BPB_FATSz16;     // 每个FAT占用扇区数
    ushort BPB_SecPerTrk;   // 每个磁道扇区数
    ushort BPB_NumHeads;    // 磁头数
    uint BPB_HiddSec;       // 隐藏扇区数
    uint BPB_TotSec32;      // 如果BPB_TotSec16是0，则在这里记录
    uchar BS_DrvNum;        // 中断13的驱动器号
    uchar BS_Reserved1;     // 未使用
    uchar BS_BootSig;       // 扩展引导标志
    uint BS_VolID;          // 卷序列号
    char BS_VolLab[11];  // 卷标，必须是11个字符，不足以空格填充
    char BS_FileSysType[8];  // 文件系统类型，必须是8个字符，不足填充空格
};
typedef struct fileTree {
    std::string name;
    std::vector<fileTree> childs;
    int size = 0;
    int childFileCnt = 0;  // 子文件数`
    int DIR_FstClus = 0;   // 簇号
    int childDirCnt = 0;   // 子目录数目
    bool isDirectory = true;
} fileTree;
/*
 *
 * 将根目录的文件信息放到数组里
 * */
void getRootEnts(int rootEntsCnt, int BPB_BytsPerSec, FILE *fp, RootEntry *re) {
    fseek(fp, 19 * BPB_BytsPerSec, SEEK_SET);
    fread(re, sizeof(RootEntry), rootEntsCnt, fp);
}

/**
 * @brief 自定义的格式化输出，同 printf
 *
 * @param format
 * @param ...
 */
void _printFormat(const char *format, ...) {
    va_list ap;            // define args pointer
    va_start(ap, format);  // point to the first argument
    char *s = new char[strlen(format)+1];
    vsprintf(s, format, ap);
    _print((const char *)s, strlen((const char *)s));
    va_end(ap);
    delete[] s;
}
std::string addZeroToChars(char *source) {
    char dist[12];
    for (int i = 0; i < 12; i++) {
        dist[i] = 0;
        if (i != 11) {
            dist[i] = source[i];
        }
    }

    std::string s = std::string(dist);
    std::regex re1("(\\.{1,2})[ ]+");
    std::smatch sm;
    if (std::regex_match(s, sm, re1)) {
        return sm.str(1);
    }

    std::regex re3("^([A-Z0-9]+)[ ]{3,}$");

    if (regex_search(s, sm, re3)) {
        return sm.str(1);
    }

    std::regex re2("^([A-Z0-9]{1,8})[ ]*([A-Z0-9]+)$");
    if (std::regex_match(s, sm, re2)) {
        return sm.str(1) + '.' + sm.str(2);
    }
    return "";
}
/*
 * 遍历一个目录，root 是根目录文件信息
 * 每个文件夹第一个子文件夹是 . 第二个是 ..
 * DIR_FstClus 是簇号
 * DIR_Attr 16  文件夹， 32 文件， 0 空
 * */
void tree(fileTree *ft, FILE *fp) {
    std::vector<fileTree> fts;
    fseek(fp, (33 - 2 + ft->DIR_FstClus) * 512, SEEK_SET);
    RootEntry re;
    for (int i = 0;; i++) {
        fread(&re, sizeof(RootEntry), 1, fp);
        if(re.DIR_Attr==15){
            continue;
        }
        fileTree newFileTreeNode;
        std::string trueName = addZeroToChars(re.DIR_Name);
        if (re.DIR_Attr == 0 ){
            break;
        }

        newFileTreeNode.name = trueName;
        newFileTreeNode.DIR_FstClus = int(re.DIR_FstClus);
        if (re.DIR_Attr == 16) {
            newFileTreeNode.isDirectory = true;
            ft->childDirCnt += 1;
        } else if (re.DIR_Attr == 32 ) {
            newFileTreeNode.size = int(re.DIR_FileSize);
            newFileTreeNode.isDirectory = false;
            newFileTreeNode.size = int(re.DIR_FileSize);
            ft->childFileCnt += 1;
        }
        ft->childs.push_back(newFileTreeNode);
    }
    for (int i = 0; i < ft->childFileCnt + ft->childDirCnt; i++) {
        if (ft->childs[i].isDirectory && ft->childs[i].name != "." &&
            ft->childs[i].name != "..")
            tree(&(ft->childs[i]), fp);
    }
}

/**
 *
 * 从 fat 表中获取下一个簇号
 * 3 个 byte 存 2 个表项
 * 簇号偶: vec[i]|((vec[i+1]&0b00001111)<<8)
 * 簇号奇: (vec[i+1]<<4)|((vec[i]&0b11110000)>>4)
 *
 * low                     high
 * --------------------------->
 * +--------+---------+--------+
 * |00000001|0010 0011|00000100|
 * |--------+----+----+--------+
 * |0011 00000001|00000100 0010|
 * +---------------------------+
 */
ushort getNext(FILE *fp, ushort DIR_FstClus) {
    char a, b;
    ushort ans = 0;
    if (DIR_FstClus % 2 == 0)
        fseek(fp, 512 + (DIR_FstClus * 1.5), SEEK_SET);
    else
        fseek(fp, 512 + ((DIR_FstClus - 1) * 1.5 + 1), SEEK_SET);
    fread(&a, sizeof(char), 1, fp);
    fread(&b, sizeof(char), 1, fp);
    if (DIR_FstClus % 2)
        ans = (b << 4) | ((a & 0b11110000) >> 4);
    else
        ans = a | ((b & 0b00001111) << 8);
    return ans;
}

void printFIle(ushort DIR_FstClus, FILE *fp, int size) {
    uchar *buf;
    if (size <= 512) {
        buf = new uchar[515];
    } else {
        buf = new uchar[512 * 3 + 3];
    }
    int offset = 0;
    do {
        fseek(fp, (33 - 2 + DIR_FstClus) * 512, SEEK_SET);
        // 3 个 byte 1 个字符, 防止被截断
        //        fread(buf,512,1,fp);
        if (size <= 512) {
            memset(buf, 0, 515);
            fread(buf, 1, 512, fp);
            _printFormat((const char *)buf);
        } else {
            if (offset == 0) memset(buf, 0, 512 * 3 + 3);
            fread(buf + 512 * offset, 512, 1, fp);
            offset += 1;
            if (offset == 3) {
                _printFormat((const char *)buf);
                offset = 0;
            }
        }
    } while ((DIR_FstClus = getNext(fp, DIR_FstClus)) < 0xFF7);
    if (offset != 0) {
        _printFormat((const char *)buf);
    }
    _printFormat("\n");
}
/**
 *
 * 广度优先遍历
 * @param ft
 * @param baseName
 */
void traverse(fileTree *ft, std::string baseName, bool l = false) {
    if (baseName.back() != '/') {
        baseName = baseName + '/';
    }
    if (!l) {
        _printFormat(baseName.c_str());
        _print(":\n", 2);
    } else {
        if (baseName != "/") {
            int childDirCnt = ft->childDirCnt >= 2 ? ft->childDirCnt - 2 : 0;

            _printFormat("%s %d %d:\n", baseName.c_str(), childDirCnt,
                         ft->childFileCnt);
            /* printf("%s %d %d:\n",baseName.c_str(),childDirCnt,
             * ft->childFileCnt);
             */
        } else {
            /* printf("%s %d
             * %d:\n",baseName.c_str(),ft->childDirCnt,ft->childFileCnt); */
            _printFormat("%s %d %d:\n", baseName.c_str(), ft->childDirCnt,
                         ft->childFileCnt);
        }
    }

    //    printf("%s %d
    //    %d:\n",baseName.c_str(),ft->childDirCnt-2,ft->childFileCnt-2);
    for (int i = 0; i < ft->childFileCnt + ft->childDirCnt; i++) {
        //        printf("%s ",ft->childs[i].name.c_str());
        if (!l)
        /* printf("%s  ",ft->childs[i].name.c_str()); */
        {
            if (ft->childs[i].isDirectory) {
                _printFormat("%s%s  ", RED, ft->childs[i].name.c_str());
            } else {
                _printFormat("%s%s  ", DEFAULT, ft->childs[i].name.c_str());
            }
        } else {
            if (ft->childs[i].name == "." || ft->childs[i].name == "..")
            /* printf("%s\n",ft->childs[i].name.c_str()); */
            {
                _printFormat("%s%s\n", RED, ft->childs[i].name.c_str());
            } else {
                if (ft->childs[i].isDirectory) {
                    int childDirCnt = ft->childs[i].childDirCnt >= 2
                                          ? ft->childs[i].childDirCnt - 2
                                          : 0;
                    /* printf("%s %d
                     * %d\n",ft->childs[i].name.c_str(),childDirCnt,ft->childs[i].childFileCnt);
                     */
                    _printFormat("%s%s %s%d %d\n", RED,
                                 ft->childs[i].name.c_str(), DEFAULT,
                                 childDirCnt, ft->childs[i].childFileCnt);
                } else {
                    /* printf("%s
                     * %d\n",ft->childs[i].name.c_str(),ft->childs[i].size); */

                    _printFormat("%s%s %d\n", DEFAULT,
                                 ft->childs[i].name.c_str(),
                                 ft->childs[i].size);
                }
            }
        }
    }
    _printFormat("\n");
    for (int i = 0; i < ft->childFileCnt + ft->childDirCnt; i++) {
        if (ft->childs[i].name != ".." && ft->childs[i].name != "." &&
            ft->childs[i].isDirectory)
            traverse(&(ft->childs[i]), baseName + ft->childs[i].name, l);
    }
}
/**
 *
 * 寻找到要遍历目录的文件结点
 * @param ft 文件结点
 * @param baseName 要遍历的目录
 * @param workDir 文件结点对应的绝对路径
 * @return 返回文件结点
 */
fileTree *ls(fileTree *ft, std::string baseName, std::string workDir) {
    if (baseName.front() != '/') {
        baseName = '/' + baseName;
    }
    if (baseName.back() != '/') {
        baseName += '/';
    }
    if (workDir.back() != '/') {
        workDir += '/';
    }
    if (workDir == baseName) {
        //        traverse(ft,baseName);
        return ft;
    }
    for (int i = 0; i < ft->childDirCnt + ft->childFileCnt; i++) {
        if (ft->childs[i].name == "." || ft->childs[i].name == "..") {
            continue;
        }
        //        if(ft.childs[i].name)
        std::regex re("^(" + workDir + ft->childs[i].name + "/).*");
        if (std::regex_match(baseName, re)) {
            return ls(&(ft->childs[i]), baseName, workDir + ft->childs[i].name);
        }
    }
    //    printf("没找见\n");
    return nullptr;
}

void handler(fileTree *ft, FILE *fp) {
    char s[512];
    std::regex CAT("^([\t ]*cat)(([ \t]+.*$)|$)");
    std::regex CAT_1("^[\t ]*cat[ \t]+([/A-Z0-9\\.]+)[ \t]*$");
    std::regex LS("(^([\t ]*ls)[ \t]*$)|((^[\t ]*ls)[\t ]+.*$)");
    std::regex EXIT("^[\t ]*exit[ \t]*$");

    std::regex LS_1(
        "^[ \t]*ls([ \t]+-l+)*([\t ]+([/0-9A-Z]+))?([\t ]+-l+)*[\t "
        "]*$");  // 其实这一个就够了 但是我不想改了
    std::regex LS_2("-l*[^ l\t]");  // 不支持的参数
    std::regex LS_4("^[\t ]*ls[\t ]+(-l+)[\t ]*$");
    std::regex LS_3("^[ \t]*ls[\t ]*$");
    while (1) {
        rewind(stdin);
        fflush(stdin);
        scanf("%[^\n]", s);
        getchar();
        std::string input(s);

        std::smatch sm;
        if (std::regex_match(input, LS)) {
            if (std::regex_match(input, LS_3)) {
                // printf("ls /\n");
                traverse(ft, "/");
            } else if (std::regex_search(input, LS_2)) {
                /* printf("不受支持参数!\n"); */
                _printFormat(INVALID_OPTION);
            } else if (std::regex_match(input, sm, LS_1)) {
                if (!sm.str(1).empty() || !sm.str(4).empty()) {
                    if (sm.str(3).empty()) {
                        traverse(ft, "/", true);
                    } else {
                        fileTree *found = ls(ft, sm.str(3), "/");
                        if (found != nullptr)
                            traverse(found, sm.str(3), true);
                        else
                            /* printf("不存在的目录\n"); */
                            _printFormat(FILE_NOT_FOUND);
                    }
                } else {
                    fileTree *found = ls(ft, sm.str(3), "/");
                    if (found != nullptr)
                        traverse(found, sm.str(3));
                    else
                        /* printf("不存在的目录\n"); */
                        _printFormat(FILE_NOT_FOUND); 
                }
            } else if (std::regex_match(input, LS_4)) {
                traverse(ft, "/", true);
            } else {
                /* printf("错误的参数\n"); */
                _printFormat(PARSE_ERROR );
            }
        } else if (std::regex_match(input, CAT)) {
            if (std::regex_match(input, sm, CAT_1)) {
                fileTree *found = ls(ft, sm.str(1), "/");
                if (found != nullptr)
                    if (!found->isDirectory)
                        printFIle(found->DIR_FstClus, fp, found->size);
                    else
                        _printFormat(NOT_A_FILE);
                else
                    /* printf("不存在的文件\n"); */
                    _printFormat(FILE_NOT_FOUND);
            } else {
                /* printf("参数错误\n"); */
                _printFormat(PARSE_ERROR);
            }
        } else if (std::regex_match(input, EXIT)) {
            break;
        } else {
            /* printf("没有这个命令\n"); */
            _printFormat(COMMAND_NOT_FOUND);
        }
    }
}

int main() {
    FILE *fp = fopen("./a.img", "rb");
    fileTree ft;
    ft.DIR_FstClus = -12;
    ft.name = "/";

    struct Fat12Header fh;
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fseek(fp, 3, SEEK_SET);
    fread(&fh, sizeof(Fat12Header), 1, fp);
    fseek(fp, 19 * 512, SEEK_SET);
    tree(&ft, fp);
    handler(&ft, fp);

    fclose(fp);
    return 0;
}
