#include <stdio.h>
#include <stdlib.h>
#include "../src/ipdb_lib.h"
#include "../src/util.c"
#include "../src/ipdb.c"
#include "../src/qqwry.c"
#include "../src/ipip.c"
#include "../src/txtdb.c"
#include "../src/qqwry_build.c"
#include "../src/patch.c"
#include "../src/cz_update.c"

static uint8_t* readfile(const char *path, uint32_t *length)
{
    uint8_t* buffer = 0;
    FILE *fp = fopen(path, "rb");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        *length = ftell(fp);
        buffer = (uint8_t*)malloc(*length);

        fseek(fp, 0, SEEK_SET);
        fread(buffer, *length, 1, fp);
        fclose(fp);
    }
    return buffer;
}

static void test_table()
{
    buffer *string_buffer = buffer_create();
    table *test = table_create(string_buffer);

    table_set_key(test, "��ͨ");
    table_set_key(test, "����");
    table_set_key(test, "����");
    table_set_key(test, "����ְҵ����ѧԺ");

    show_table_key(test);
    table_release(test);
    buffer_release(string_buffer);
}

static void test_read_qqwry()
{
    uint32_t length = 0;
    uint8_t *buffer = readfile("qqwry.dat", &length);
    ipdb *db = ipdb_create(&qqwry_handle, buffer, length, NULL);
    if(db)
    {
        ipdb_item item;
        if( ipdb_find(db, &item, "112.121.182.84") )
        {
            char ip1[16];
            char ip2[16];

            char *ip1_t = ip2str(ip1, sizeof(ip1), item.lower);
            char *ip2_t = ip2str(ip2, sizeof(ip2), item.upper);
            printf("%s %s %s %s\n", ip1_t, ip2_t, item.zone, item.area);
        }
        printf("%d %d\n", db->count, db->date);
        ipdb_dump(db, "qqwry.txt");
        ipdb_release(db);
    }

    if(buffer) free(buffer);
}

static void test_read_ipip()
{
    uint32_t length = 0;
    uint8_t *buffer = readfile("17monipdb.datx", &length);
    ipdb *db = ipdb_create(&ipip_handle, buffer, length, NULL);
    if(db)
    {
        ipdb_item item;
        if( ipdb_find(db, &item, "112.121.182.84") )
        {
            char ip1[16];
            char ip2[16];

            char *ip1_t = ip2str(ip1, sizeof(ip1), item.lower);
            char *ip2_t = ip2str(ip2, sizeof(ip2), item.upper);
            printf("%s %s %s %s\n", ip1_t, ip2_t, item.zone, item.area);
        }

        printf("%d %d\n", db->count, db->date);
        ipdb_dump(db, "ipip.txt");
        ipdb_release(db);
    }

    if(buffer) free(buffer);
}

static void test_read_txt()
{
    uint32_t length = 0;
    uint8_t *buffer = readfile("1.txt", &length);
    ipdb *db = ipdb_create(&txtdb_handle, buffer, length, NULL);
    if(db)
    {
        ipdb_item item;
        if( ipdb_find(db, &item, "112.121.182.84") )
        {
            char ip1[16];
            char ip2[16];

            char *ip1_t = ip2str(ip1, sizeof(ip1), item.lower);
            char *ip2_t = ip2str(ip2, sizeof(ip2), item.upper);
            printf("%s %s %s %s\n", ip1_t, ip2_t, item.zone, item.area);
        }
        printf("%d %d\n", db->count, db->date);
        ipdb_dump(db, "2.txt");
        ipdb_release(db);
    }

    if(buffer) free(buffer);
}

static void test_build_qqwry()
{
    uint32_t length = 0;
    uint8_t *buffer = readfile("qqwry.dat", &length);
    ipdb *db = ipdb_create(&qqwry_handle, buffer, length, NULL);

    if (db)
    {
        qqwry_build(db, "qqwry new.dat");
        ipdb_release(db);
    }

    if(buffer) free(buffer);

}

static void test_build_patch()
{
    uint32_t length1 = 0;
    uint32_t length2 = 0;
    uint8_t *buffer1 = readfile("qqwry 520.dat", &length1);
    uint8_t *buffer2 = readfile("qqwry 525.dat", &length2);
    ipdb *db1 = ipdb_create(&qqwry_handle, buffer1, length1, NULL);
    ipdb *db2 = ipdb_create(&qqwry_handle, buffer2, length2, NULL);

    if(db1 && db2)
    {
        make_patch(db1, db2);
    }

    if(db1) ipdb_release(db1);
    if(db2) ipdb_release(db2);

    if(buffer1) free(buffer1);
    if(buffer2) free(buffer2);
}

static void test_apply_patch()
{
    uint32_t length1 = 0;
    uint32_t length2 = 0;
    uint8_t *buffer1 = readfile("qqwry 520.dat", &length1);
    uint8_t *buffer2 = readfile("20140520-20140525.db", &length2);

    ipdb *db1 = ipdb_create(&qqwry_handle, buffer1, length1, NULL);
    if(db1)
    {
        ipdb* db = apply_patch(db1, buffer2, length2);
        if(db)
        {
            printf("apply_patch %d\n", db->date);
            qqwry_build(db, "qqwry patch.dat");
            ipdb_dump(db, "525 new.txt");
            ipdb_release(db);
        }
        ipdb_release(db1);
    }

    if(buffer1) free(buffer1);
    if(buffer2) free(buffer2);
}

static void test_cz_update()
{
    uint32_t length1 = 0;
    uint32_t length2 = 0;
    uint8_t *buffer1 = readfile("copywrite.rar", &length1);
    uint8_t *buffer2 = readfile("qqwry.rar", &length2);

    const cz_update *update = parse_cz_update(buffer1, length1);
    if(update)
    {
        uint8_t *qqwry = decode_cz_update(update, buffer2, length2, &length1);
        if(qqwry)
        {
            printf("qqwry %d\n", length1);
            free(qqwry);
        }
        printf("%d\n", get_cz_update_date(update));
    }

    if(buffer1) free(buffer1);
    if(buffer2) free(buffer2);
}

static void ipip_to_qqwry()
{
    uint32_t length = 0;
    uint8_t *buffer = readfile("17monipdb.datx", &length);
    ipdb *db = ipdb_create(&ipip_handle, buffer, length, NULL);
    if(db)
    {
        printf("����ת����\n");
        qqwry_build(db, "qqwry.dat");
        printf("ת�����\n");
        /*
        ipdb_item item;
        if( ipdb_find(db, &item, "112.121.182.84") )
        {
            char ip1[16];
            char ip2[16];

            char *ip1_t = ip2str(ip1, sizeof(ip1), item.lower);
            char *ip2_t = ip2str(ip2, sizeof(ip2), item.upper);
            printf("%s %s %s %s\n", ip1_t, ip2_t, item.zone, item.area);
        }

        printf("%d %d\n", db->count, db->date);
        ipdb_dump(db, "ipip.txt");
        */
        ipdb_release(db);
    }
    else
    {
        printf("��17monipdb.datxʧ��\n");
    }

    if(buffer) free(buffer);
}
int main()
{
    test_read_ipip();
    // ipip_to_qqwry();
    // test_table();
    // test_read_qqwry();
    // test_build_qqwry();
    // test_build_patch();
    // test_apply_patch();
    // test_read_txt();
    // test_cz_update();
    return 0;
}
