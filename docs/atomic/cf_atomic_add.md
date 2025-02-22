[](../header.md ':include')

# cf_atomic_add

Category: [atomic](/api_reference?id=atomic)  
GitHub: [cute_multithreading.h](https://github.com/RandyGaul/cute_framework/blob/master/include/cute_multithreading.h)  
---

Atomically adds `addend` to `atomic` and returns the old value from `atomic`.

```cpp
int cf_atomic_add(CF_AtomicInt* atomic, int addend);
```

Parameters | Description
--- | ---
atomic | The integer to atomically manipulate.
addend | A value to atomically add to `atomic`.

## Remarks

Atomics are an advanced topic. You've been warned! Beej has a [good article on atomics](https://beej.us/guide/bgc/html/split/chapter-atomics.html).

## Related Pages

[cf_atomic_zero](/atomic/cf_atomic_zero.md)  
[cf_atomic_ptr_cas](/atomic/cf_atomic_ptr_cas.md)  
[cf_atomic_set](/atomic/cf_atomic_set.md)  
[cf_atomic_get](/atomic/cf_atomic_get.md)  
[cf_atomic_cas](/atomic/cf_atomic_cas.md)  
[cf_atomic_ptr_set](/atomic/cf_atomic_ptr_set.md)  
[cf_atomic_ptr_get](/atomic/cf_atomic_ptr_get.md)  
