# Architecture Cleanup - Revised Approach

**Date:** 2026-03-12  
**Approach:** Rename "_new" functions, keep old implementations as fallbacks

---

## Revised Strategy

Instead of removing old implementations, we'll:
1. Rename `*_new()` functions to remove the "_new" suffix
2. Keep old implementations in place as automatic fallbacks
3. This preserves backward compatibility while cleaning up naming

---

## Functions to Rename

1. `cpu_calculate_new()` → Already has fallback logic, just needs renaming
2. `update_node_world_position_according_to_position_array_new()` → Has fallback
3. `update_link_position_new()` → Has fallback  
4. `rotate_to_face_player_new()` → Has fallback
5. `generate_objects_for_node_and_link_new()` → Has fallback

---

## Benefits of This Approach

✅ Preserves old code for reference/debugging
✅ Automatic fallback if new systems fail
✅ Minimal risk - just renaming
✅ Clear which is current (no suffix) vs legacy (old name)
✅ Easy to test - fallbacks still work

---

## Status

Reverted previous changes. Ready to implement simple rename approach.
