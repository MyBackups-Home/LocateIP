#include "ipdb.h"

/*
https://www.ipip.net/ip.html
*/

#pragma pack(push, 1)
typedef struct
{
    uint32_t upper;
    uint8_t buffer[5];
    // uint32_t offset:24;
    // uint32_t length:16;
} ipip_item;
#pragma pack(pop)

#define IPIP_ITEM_SIZE 9
#define IPIP_INDEX_SIZE 4*256*256

static uint32_t swap32(uint32_t n)
{
    return ((n<<24)|((n<<8)&0x00FF0000)|((n>>8)&0x0000FF00)|(n>>24));
}

typedef int(*compare_cb)(const uint8_t *a, const uint8_t *b);
static int array_unique(uint8_t *array, int length, int size, compare_cb cmp)
{
	uint8_t *begin = array;
	uint8_t *end = array + (length - 1) * size;

	while (begin<end)
	{
		uint8_t *current = begin + size;
		while (current <= end)
		{
			if (cmp(current, begin))
			{
				memmove(current, current + size, end - current);
				end -= size;
			}
			else current += size;
		}
		begin += size;
	}

	return (int)(end - array + size) / size;
}

typedef char* string;
int is_equal(const uint8_t *a, const uint8_t *b)
{
    return strcmp(*(string*)a, *(string*)b)==0;
}

static bool ipip_iter(const ipdb *db, ipdb_item *item, uint32_t index)
{
    static char buf[1024];
    static char area[1024];

    if(index<db->count)
    {
        ipip_item *ptr = (ipip_item*)(db->buffer + 4 + IPIP_INDEX_SIZE);

        uint8_t *buffer = ptr[index].buffer;
        uint32_t offset = buffer[0] | buffer[1] << 8 | buffer[2] << 16;
        uint16_t length = buffer[3] << 8 | buffer[4];

        const char *text = (const char*)db->buffer + 4 + IPIP_INDEX_SIZE + db->count*IPIP_ITEM_SIZE + offset;

        item->lower = index==0?0:(swap32(ptr[index-1].upper)+1);
        item->upper = swap32(ptr[index].upper);

        memcpy(buf, text, length);
        buf[length] = 0;

        string a = buf;
        string b = strchr(a, '\t');
        string c = strchr(b + 1, '\t');
        string d = strchr(c + 1, '\t');

        *b++ = 0;
        *c++ = 0;
        *d++ = 0;

        area[0] = 0;

        string list[] = {a, b, c, d};
        int count = array_unique((uint8_t*)list, 4, sizeof(string), is_equal);
        int i = 1;
        for (; i < count; i++)
        {
            strcat(area, list[i]);
        }

        item->zone = buf;
        item->area = area;
        return true;
    }
    return false;
}

static bool ipip_find(const ipdb *db, ipdb_item *item, uint32_t ip)
{
    uint32_t low = 0;
    uint32_t high = db->count;
    ipip_item *ptr = (ipip_item*)(db->buffer + 4 + IPIP_INDEX_SIZE);
    while ( low < high )
    {
        uint32_t mid = low + (high - low)/2;
        if( ip > swap32(ptr[mid].upper) )
            low = mid + 1;
        else
            high = mid;
    }
    return ipip_iter(db, item, low);
}

static bool ipip_init(ipdb* db)
{
    if(db->length>=4 && sizeof(ipip_item)==IPIP_ITEM_SIZE)
    {
        uint32_t *pos = (uint32_t*)db->buffer;
        uint32_t index_length = swap32(*pos);

        ipdb_item item;
        uint32_t year = 0, month = 0, day = 0;

        db->count = (index_length - 4 - IPIP_INDEX_SIZE - IPIP_INDEX_SIZE)/IPIP_ITEM_SIZE;

        if(ipip_iter(db, &item, db->count-1))
        {
            if( sscanf(item.area, "%4d%2d%2d", &year, &month, &day)!=3 ) /* ipip数据库 */
            {
                year = 1899, month = 12, day = 30; /* 未知数据库 */
            }
        }
        db->date = year*10000 + month*100 + day;
    }
    return db->count!=0;
}

const ipdb_handle ipip_handle = {ipip_init, ipip_iter, ipip_find, NULL};
