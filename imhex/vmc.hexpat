import std.io;
import std.mem;

u32 ALLOC_OFFSET @ 0x34 [[hidden]];
u32 CLUSTER_SIZE @ 0x154 [[hidden]];
u32 FAT_ENTRIS_PER_CLUSTER @ 0x158 [[hidden]];
u32 CURRENT_FAT_INDEX= 0;

fn GetFatIndex() {
    u32 idx = CURRENT_FAT_INDEX;
    CURRENT_FAT_INDEX = CURRENT_FAT_INDEX + 1;
    return idx;
};

fn TransformAbsoluteClusterPointer(u32 cluster) {
    return cluster * CLUSTER_SIZE - cluster;
};

enum FSEntryMode : u16 {
    EM_READ = 0x1,
    EM_WRITE = 0x2,
    EM_EXECUTE = 0x4,
    EM_PROTECTED = 0x8,
    EM_FILE = 0x10,
    EM_DIRECTORY = 0x20,
    EM_POCKETSTATION = 0x800,
    EM_PLAYSTATION = 0x1000,
    EM_HIDDEN = 0x2000,
    EM_EXISTS = 0x8000
};

struct DateTime {
    u8 __reserved [[hidden]];
    u8 sec [[name("Seconds")]];
    u8 min [[name("Minutes")]];
    u8 hour [[name("Hour")]];
    u8 day [[name("Day")]];
    u8 month [[name("Month")]];
    u16 year [[name("Year")]];
};

struct FSEntry {
    FSEntryMode mode [[name("Mode")]];
    u16 __unused [[hidden]];
    u32 length [[name("Length")]];
    DateTime created [[name("Crated At (japan time)")]];
    u32 cluster [[name("Cluster")]];
    u32 dirEntry [[name("Directory Entry")]];
    DateTime modified [[name("Modified At (japan time)")]];
    u32 attr [[name("Attributes")]];
    u32 __unused2[7] [[hidden]];
    char name[32] [[name("Name")]];
    u8 __unused3[416] [[hidden]];
};

struct FatDataPointer {
    u32 temp;
};

bitfield FatEntry {
    next: 24 [[name("Next Cluster")]];
    flags: 8 [[name("Flags")]];
};

struct FAT {
    FatEntry clusters[FAT_ENTRIS_PER_CLUSTER] [[inline]];
} [[inline]];

struct FatPointer {
    FAT * fat: u32 [[name("FAT"), pointer_base("TransformAbsoluteClusterPointer")]];
} [[inline]];

struct IfcTable {
    FatPointer fats[FAT_ENTRIS_PER_CLUSTER] [[inline]];
} [[inline]];

struct IfcPointer {
    if(std::mem::read_unsigned($, 4) == 0) {
        u32 ifc [[hidden]];
    } else {
        IfcTable * ifc: u32 [[name("IFC"), pointer_base("TransformAbsoluteClusterPointer")]];
    }
} [[inline]];

struct Superblock {
    char magic[28] [[name("Magic")]];
    char version[12] [[name("Version")]];
    s16 pageSize [[name("Page Size")]];
    u16 pagesPerCluster [[name("Pages per Cluster")]];
    u16 pagesPerBlock [[name("Pages per Block")]];
    u16 __unused [[hidden]];
    u32 clustersPerCard [[name("Clusters per Card")]];
    u32 allocOffset [[name("Alloc Offset (in clusters)")]];
    u32 allocEnd [[name("Alloc End (in clusters)")]];
    u32 rootDirCluster [[name("Root Directory Cluster")]];
    u32 backupBlock1 [[name("Backup Block #1")]];
    u32 backupBlock2 [[name("Backup Block #2")]];
    u8 __unused2[8] [[hidden]];
    IfcPointer ifcList[32] [[name("IFC List")]];
    u32 badBlockList[32] [[name("Bad Block List")]];
    u8 cardType [[name("Card Type")]];
    u8 cardFlags [[name("Card Flags")]];
    u16 __unused3 [[hidden]];
    u32 clusterSize [[name("Cluster Size")]];
    u32 fatEntriesPerCluster [[name("FAT Entries per Cluster")]];
    u32 clustersPerBlock [[name("Clusters per Block")]];
    s32 cardFormat [[name("Card Format")]];
    u32 rootDirCluster2 [[name("Root Directory Cluster (2)")]];
    u32 __unknown1 [[hidden]];
    u32 __unknown2 [[hidden]];
    u32 maxAllocatableClusters [[name("Max Allocatable Clusters")]];
    u32 __unknown3 [[hidden]];
    u32 __unknown4 [[hidden]];
    s32 __unknown5 [[hidden]];
};

Superblock superblock @ 0x0 [[name("Memory Card")]];
