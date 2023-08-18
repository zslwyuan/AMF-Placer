// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "./api-build_p.h"
#include "./zoneallocator_p.h"
#include "./zonelist_p.h"

// ============================================================================
// [BLZoneList - Unit Tests]
// ============================================================================

#ifdef BL_TEST
class MyListNode : public BLZoneListNode<MyListNode> {};

UNIT(zone_list, -5) {
  BLZoneAllocator zone(4096);
  BLZoneList<MyListNode> list;

  MyListNode* a = zone.newT<MyListNode>();
  MyListNode* b = zone.newT<MyListNode>();
  MyListNode* c = zone.newT<MyListNode>();
  MyListNode* d = zone.newT<MyListNode>();

  INFO("Append / Unlink");

  // []
  EXPECT(list.empty() == true);

  // [A]
  list.append(a);
  EXPECT(list.empty() == false);
  EXPECT(list.first() == a);
  EXPECT(list.last() == a);
  EXPECT(a->prev() == nullptr);
  EXPECT(a->next() == nullptr);

  // [A, B]
  list.append(b);
  EXPECT(list.first() == a);
  EXPECT(list.last() == b);
  EXPECT(a->prev() == nullptr);
  EXPECT(a->next() == b);
  EXPECT(b->prev() == a);
  EXPECT(b->next() == nullptr);

  // [A, B, C]
  list.append(c);
  EXPECT(list.first() == a);
  EXPECT(list.last() == c);
  EXPECT(a->prev() == nullptr);
  EXPECT(a->next() == b);
  EXPECT(b->prev() == a);
  EXPECT(b->next() == c);
  EXPECT(c->prev() == b);
  EXPECT(c->next() == nullptr);

  // [B, C]
  list.unlink(a);
  EXPECT(list.first() == b);
  EXPECT(list.last() == c);
  EXPECT(a->prev() == nullptr);
  EXPECT(a->next() == nullptr);
  EXPECT(b->prev() == nullptr);
  EXPECT(b->next() == c);
  EXPECT(c->prev() == b);
  EXPECT(c->next() == nullptr);

  // [B]
  list.unlink(c);
  EXPECT(list.first() == b);
  EXPECT(list.last() == b);
  EXPECT(b->prev() == nullptr);
  EXPECT(b->next() == nullptr);
  EXPECT(c->prev() == nullptr);
  EXPECT(c->next() == nullptr);

  // []
  list.unlink(b);
  EXPECT(list.empty() == true);
  EXPECT(list.first() == nullptr);
  EXPECT(list.last() == nullptr);
  EXPECT(b->prev() == nullptr);
  EXPECT(b->next() == nullptr);

  INFO("Prepend / Unlink");

  // [A]
  list.prepend(a);
  EXPECT(list.empty() == false);
  EXPECT(list.first() == a);
  EXPECT(list.last() == a);
  EXPECT(a->prev() == nullptr);
  EXPECT(a->next() == nullptr);

  // [B, A]
  list.prepend(b);
  EXPECT(list.first() == b);
  EXPECT(list.last() == a);
  EXPECT(b->prev() == nullptr);
  EXPECT(b->next() == a);
  EXPECT(a->prev() == b);
  EXPECT(a->next() == nullptr);

  INFO("InsertAfter / InsertBefore");

  // [B, A, C]
  list.insertAfter(a, c);
  EXPECT(list.first() == b);
  EXPECT(list.last() == c);
  EXPECT(b->prev() == nullptr);
  EXPECT(b->next() == a);
  EXPECT(a->prev() == b);
  EXPECT(a->next() == c);
  EXPECT(c->prev() == a);
  EXPECT(c->next() == nullptr);

  // [B, D, A, C]
  list.insertBefore(a, d);
  EXPECT(list.first() == b);
  EXPECT(list.last() == c);
  EXPECT(b->prev() == nullptr);
  EXPECT(b->next() == d);
  EXPECT(d->prev() == b);
  EXPECT(d->next() == a);
  EXPECT(a->prev() == d);
  EXPECT(a->next() == c);
  EXPECT(c->prev() == a);
  EXPECT(c->next() == nullptr);

  INFO("PopFirst / Pop");

  // [D, A, C]
  EXPECT(list.popFirst() == b);
  EXPECT(b->prev() == nullptr);
  EXPECT(b->next() == nullptr);

  EXPECT(list.first() == d);
  EXPECT(list.last() == c);
  EXPECT(d->prev() == nullptr);
  EXPECT(d->next() == a);
  EXPECT(a->prev() == d);
  EXPECT(a->next() == c);
  EXPECT(c->prev() == a);
  EXPECT(c->next() == nullptr);

  // [D, A]
  EXPECT(list.pop() == c);
  EXPECT(c->prev() == nullptr);
  EXPECT(c->next() == nullptr);

  EXPECT(list.first() == d);
  EXPECT(list.last() == a);
  EXPECT(d->prev() == nullptr);
  EXPECT(d->next() == a);
  EXPECT(a->prev() == d);
  EXPECT(a->next() == nullptr);
}
#endif
