#include "block.h"

#include <assert.h>
#include <stddef.h>

static const char *const block_type_to_string[] = {
#define BLOCK(name, name_string, top_color, color) \
    [BLOCK_TYPE_##name] = name_string,
    BLOCKS
#undef BLOCK
};

void block_init(Block *const block, const BlockType type) {
    assert(block != NULL);
    block->type = type;
}

inline const char *block_get_name(const BlockType type) {
    return block_type_to_string[type];
}
