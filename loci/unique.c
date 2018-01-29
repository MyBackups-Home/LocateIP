#include "util.h"

typedef struct
{
    buffer *record_buffer;
    buffer *string_buffer;
} ipdb_unique_extend;

typedef struct
{
    uint32_t lower;
    uint32_t upper;
    uint32_t zone;
    uint32_t area;
} ipdb_unique_item;

static bool unique_iter(const ipdb *db, ipdb_item *item, uint32_t index)
{
    if(index<db->count)
    {
        ipdb_unique_extend *extend = (ipdb_unique_extend *)db->extend;
        ipdb_unique_item *items = (ipdb_unique_item *)buffer_get(extend->record_buffer);
        item->lower = items[index].lower;
        item->upper = items[index].upper;
        item->zone = (char *)buffer_get(extend->string_buffer) + items[index].zone;
        item->area = (char *)buffer_get(extend->string_buffer) + items[index].area;
        return true;
    }
    return false;
}

static bool unique_init(ipdb * db)
{
    buffer *record_buffer = buffer_create();
    buffer *string_buffer = buffer_create();

    ipdb_unique_extend *extend = (ipdb_unique_extend *)calloc(1, sizeof(ipdb_unique_extend));
    extend->record_buffer = record_buffer;
    extend->string_buffer = string_buffer;

    db->extend = extend;
    return true;
}

static bool unique_quit(ipdb* db)
{
    ipdb_unique_extend *extend = (ipdb_unique_extend *)db->extend;
    buffer_release(extend->record_buffer);
    buffer_release(extend->string_buffer);
    free(extend);
    return true;
}

const ipdb_handle unique_handle = {unique_init, unique_iter, NULL, unique_quit};

ipdb* make_unique(const ipdb *ctx)
{
    ipdb *db = ipdb_create(&unique_handle, 0, 0, NULL);
    if(db)
    {
        ipdb_unique_extend *extend = (ipdb_unique_extend *)db->extend;

        ipdb_iter iter = {ctx, 0};
        ipdb_item item;

        uint32_t last_zone = 0;
        uint32_t last_area = 0;
        while(ipdb_next(&iter, &item))
        {
            if (last_area != 0 &&
                strcmp(item.zone, (char *)buffer_get(extend->string_buffer) + last_zone) == 0 &&
                strcmp(item.area, (char *)buffer_get(extend->string_buffer) + last_area) == 0)
            {
                ipdb_unique_item *items = (ipdb_unique_item *)buffer_get(extend->record_buffer);
                items[db->count - 1].upper = item.upper;
            }
            else
            {
                uint32_t zone = buffer_append(extend->string_buffer, item.zone, (uint32_t)(strlen(item.zone) + 1));
                uint32_t area = buffer_append(extend->string_buffer, item.area, (uint32_t)(strlen(item.area) + 1));

                ipdb_unique_item unique_item;
                unique_item.lower = item.lower;
                unique_item.upper = item.upper;
                unique_item.zone = zone;
                unique_item.area = area;

                last_zone = zone;
                last_area = area;

                buffer_append(extend->record_buffer, &unique_item, sizeof(ipdb_unique_item));
                db->count++;
            }
        }
    }
    return db;
}
