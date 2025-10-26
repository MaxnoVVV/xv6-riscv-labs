#ifndef PAGEINFO_H
#define PAGEINFO_H

#define PAGEINFO_PRINT_TABLE     (1 << 0)
#define PAGEINFO_SHOW_DIRTY      (1 << 1)
#define PAGEINFO_SHOW_ACCESSED   (1 << 2)
#define PAGEINFO_CLEAR_DIRTY     (1 << 3)
#define PAGEINFO_CLEAR_ACCESSED  (1 << 4)

#define PAGEINFO_VALID_MASK (PAGEINFO_PRINT_TABLE | PAGEINFO_SHOW_DIRTY | \
                              PAGEINFO_SHOW_ACCESSED | PAGEINFO_CLEAR_DIRTY | \
                              PAGEINFO_CLEAR_ACCESSED)

#endif /* PAGEINFO_H */
