#include "block.h"

#include <assert.h>
#include <stddef.h>

static const char *const block_type_to_string[] = {
#define BLOCK(name, name_string, ...) [BLOCK_TYPE_##name] = name_string,
    BLOCKS
#undef BLOCK
};

void block_init(Block *const block, const BlockType type) {
    assert(block != NULL);
    assert(type < BLOCK_TYPE_COUNT && "invalid block type");
    block->type = type;
}

inline const char *block_get_name(const BlockType type) {
    assert(type < BLOCK_TYPE_COUNT && "invalid block type");
    return block_type_to_string[type];
}
