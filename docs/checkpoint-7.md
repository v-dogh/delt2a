# Checkpoint 7: inherited style compilation fix

This is a focused fix on top of checkpoint 6. The native OpenAI-compatible adapter remains pending.

The style `chained` concept tested access to private `last_offset_` metadata. When that test failed, the parent was treated as having no styles, so derived property IDs restarted at zero. In VirtualBox and VerticalSwitch this caused inherited layout, container, option and callback setters to resolve to unrelated fields or fail with `Invalid Property`.

UniversalAccessInterface now exposes a public `uai_chain` type marker. Detection uses that marker, and the existing friend helper reads the private offset from the identified UAI. Storage and public setter/getter signatures are unchanged. No C++ standard downgrade, disabled checks or dependency changes are required.

The new `d2_styles` regression evaluates chain detection outside the friend context before including the derived widget header. It checks offsets, inherited types, field identity and values through three levels, and instantiates the DebugBox setter patterns. The test fails against checkpoint 6's header and passes with the fix.

See the bundle's validation logs for build configurations and results. Replace the source or apply `changes-from-checkpoint-6.patch` from the delt2a directory, then rebuild all consumers of the changed header. Numeric property IDs are implementation details and should not be persisted across builds.
