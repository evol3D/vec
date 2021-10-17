/*!
 * \file vec.h
 */
#ifndef VEC_HEADER
#define VEC_HEADER

#ifdef VEC_DLL
    #if defined(_WINDOWS) || defined(_WIN32)
        #if defined (VEC_IMPL)
            #define VEC_API __declspec(dllexport)
        #else
            #define VEC_API __declspec(dllimport)
        #endif
    #elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        #if defined (VEC_IMPL)
            #define VEC_API __attribute__((visibility("default")))
        #else
            #define VEC_API
        #endif
    #endif
#else
    #define VEC_API
#endif

#ifdef _WIN32
#define __vec_align(x) __declspec(align(x))
#else
#define __vec_align(x) __attribute__((aligned(x)))
#endif

#ifndef VEC_INIT_CAP
/*!
 * \brief Initial capacity that is first reserved when a vector is initialized
 */
#define VEC_INIT_CAP 8
#endif

#ifndef VEC_GROWTH_RATE
/*!
 * \brief Rate at which a vector grows whenever a resize is needed
 */
#define VEC_GROWTH_RATE 3 / 2
#endif

#include <stddef.h>

typedef void *vec_t;
typedef void *svec_t;

typedef enum {
  VEC_ERR_NONE = 0,

  VEC_ERR_OOM = -1,

} vec_error_t;

/*!
 * \brief For the sake of readability
 * \details Sample usage:
 * ```
 * vec(int) v = vec_init(int, 0, 0);
 * ```
 */
#define vec(T) T*

/*!
 * \brief For the sake of readability
 * \details Sample usage:
 * ```
 * svec(int) v = svec_init(int, { 0, 0 });
 * ```
 */
#define svec(T) T*

/*!
 * \brief Signature of a function that copies data from one address to another.
 * \details For copying elements of type `T`, an equivalent signature should be:
 * ```
 * void T_copy(T *dst, const T *src)
 * ```
 * \param dst Address at which the data should be copied
 * \param src Address from which the data should be copied
 */
typedef void (*elem_copy)(void *dst, const void *src);

/*!
 * \brief Signature of a function that destroys data referenced by a pointer
 * \details For destroying elements of type `T`, an equivalent signature should be:
 * ```
 * void T_destr(T *d)
 * ```
 *
 * \param d Pointer to data that should be destroyed
 */
typedef void (*elem_destr)(void *d);

//! Metadata that is stored with a vector. Unique to each vector.
struct vec_meta_t {
  //! The number of elements in the vector.
  size_t length;
  //! The maximum length of the vector before it needs to be resized.
  size_t capacity;
  //! The size (in bytes) of memory that each element takes.
  size_t elemsize;

  enum {
      VEC_ALLOCATION_TYPE_STACK,
      VEC_ALLOCATION_TYPE_HEAP
  } allocationType;

  //! If set, the vector uses this function to copy new values into the vector.
  elem_copy copy_fn;
  //! If set, the vector uses this function to destroy stored values.
  elem_destr destr_fn;
};

/*!
 * \param elemsize Memory (in bytes) that should be reserved per element in the vector
 * \param copy A function pointer to the function that should be used to copy an element
 * to and from the vector. Can be NULL
 * \param destr A function pointer to the function that should be used to destroy a
 * vector element. Can be NULL
 *
 * \returns A vector object
 */
VEC_API vec_t
vec_init_impl(
  size_t elemsize, 
  elem_copy copy, 
  elem_destr destr);

/*!
 * \brief Syntactic sugar for `vec_init_impl()`
 * \details Sample usage:
 * ```
 * vec_init(int);                   // vec_init_impl(sizeof(int), NULL, NULL);
 * vec_init(int, fn_destr);         // vec_init_impl(sizeof(int), NULL, fn_destr);
 * vec_init(int, fn_cpy, fn_destr); // vec_init_impl(sizeof(int), fn_cpy, fn_destr);
 * ```
 */
#define __vec_vargs_narg(...) __vec_vargs_narg_impl(__VA_ARGS__, __vec_vargs_rseq_n())
#define __vec_vargs_narg_impl(...) __vec_vargs_arg_n(__VA_ARGS__)
#define __vec_vargs_arg_n(_1, _2, _3, N, ...) N
#define __vec_vargs_rseq_n() 3, 2, 1, 0

#define __vec_cat_impl(a,b) a##b
#define __vec_cat(a,b) __vec_cat_impl(a,b)

#define __vec_arr_size(...) sizeof(__VA_ARGS__)/sizeof((__VA_ARGS__)[0])

#define vec_init(...) __vec_cat(__vec_init_, __vec_vargs_narg(__VA_ARGS__))(__VA_ARGS__)
#define __vec_init_1(type) vec_init_impl(sizeof(type), NULL, NULL)
#define __vec_init_2(type, destr) vec_init_impl(sizeof(type), NULL, destr)
#define __vec_init_3(type, cpy, destr) vec_init_impl(sizeof(type), cpy, destr)

#define svec_init(type, ...) __svec_init_impl(type, __vec_arr_size((type[])__VA_ARGS__), __VA_ARGS__)
#define svec_init_w_cap(type, cap) __svec_init_w_cap_impl(type, cap)

#define __svec_init_impl(type, size, ...)                                 \
		(svec(type))&((struct {                                           \
		  struct vec_meta_t meta;                                         \
	 	  type data[size];                                                \
	    }) {                                                              \
	  	  .meta.length = size,                                            \
	  	  .meta.capacity = size,                                          \
	  	  .meta.elemsize = sizeof(type),                                  \
	  	  .meta.allocationType = VEC_ALLOCATION_TYPE_STACK,            \
          .meta.copy_fn = 0,                                              \
          .meta.destr_fn = 0,                                             \
	  	  .data = __VA_ARGS__                                             \
	    }).data

#define __svec_init_w_cap_impl(type, cap)                               \
		(svec(type))&((struct {                                                       \
		  struct vec_meta_t meta;                                         \
	 	  type data[cap];                                                \
	    }) {                                                              \
	  	  .meta.length = 0,                                               \
	  	  .meta.capacity = cap,                                          \
	  	  .meta.elemsize = sizeof(type),                                  \
	  	  .meta.allocationType = VEC_ALLOCATION_TYPE_STACK,            \
          .data = { 0 }                                                   \
	    }).data

/*!
 * \param v The vector that we want an iterator for
 *
 * \returns A pointer to the first element in a vector
 */
VEC_API void *
vec_iter_begin(
  vec_t v);

/*!
 * \param v The vector that we want an iterator for
 *
 * \returns A pointer to the memory block right after the last element in the vector
 */
VEC_API void *
vec_iter_end(
  vec_t v);

/*!
 * \brief A function that increments an iterator to make it point to the next
 * element in the vector
 *
 * \param v The vector that is being iterated over
 * \param iter Reference to the iterator that is being incremented
 */
VEC_API void
vec_iter_next(
  vec_t v, 
  void **iter);

#define vec_foreach(ref, vec) \
  for(ref = vec_iter_begin(vec); (void*)ref < vec_iter_end(vec); vec_iter_next(vec, (void**)&ref))

/*!
 * \brief A function that destroys a vector object. If a destructor function was
 * passed while initializing the vector, then this function is called on every
 * element before all reserved memory is freed.
 *
 * *Note*: For stack-allocated vectors (`svec`), destructors are called for 
 * elements but no memory is freed.
 *
 * \param v The vector that is being destroyed
 */
VEC_API void
vec_fini(
  vec_t v);

/*!
 * \brief A function that copies a value to the end of a vector. If a copy
 * function was passed while initializing the vector, then this function is
 * called to copy the new element into the vector. Otherwise, memcpy is used
 * with a length of `vec_meta.elemsize`. If a resize is needed but fails due to
 * 'OOM' issues, then the vector is left unchanged and VEC_ERR_OOM is returned.
 *
 * For `svec`, as long as the capacity is more than the current size, a push
 * operation is permitted. Otherwise, the operation is treated as an OOM.
 *
 * \param v Reference to the vector object
 * \param val A pointer to the element that is to be copied to the end of the
 * vector
 *
 * \returns The index of the element that was just pushed. If the operation
 * failed, a non-zero (vec_error_t) value is returned.
 */
VEC_API int
vec_push(
  vec_t *v, 
  void *val);

/*!
 * \brief A function that appends the elements of an array to the end of a
 * vector.  If a resize is needed but fails due to 'OOM' issues, then the
 * vector is left unchanged and a VEC_ERR_OOM is returned.
 *
 * For `svec`, as long as the capacity is more than the current size by the
 * desired amount, an append operation is permitted. Otherwise, the operation 
 * is treated as an OOM.
 *
 * *NOTE* The vector's copy function is not used; this is merely a memcpy
 * operation. If a deep copy is needed, individually pushing the elements of
 * the array is the way to go.
 *
 * \param v Reference to the vector object
 * \param arr A pointer to the array that is to be copied to the end of the
 * vector
 * \param size Number of elements in the array
 *
 * \returns The index of the first element that was appended to the vector. If 
 * the operation failed, a non-zero (vec_error_t) value is returned.
 */
VEC_API int
vec_append(
  vec_t *v,
  void **arr,
  size_t size);

/*!
 * \brief A function that copies the value at the end of a vector and removes
 * it from the vector. If a copy function was passed while initializing the 
 * vector, then this function is used. Otherwise, memcpy is used with a length 
 * of `vec_meta.elemsize`
 *
 * \param v Reference to the vector object
 * \param out A pointer to the memory block at which the popped element will be
 * copied. If NULL is passed, then the element is destructed. Otherwise, the
 * element is copied to `out` and the receiving code is responsible for its
 * destruction.
 * 
 * \returns An error code. If the operation was successful, then `VEC_ERR_NONE` 
 * is returned.
 */
VEC_API vec_error_t
vec_pop(
  vec_t *v, 
  void *out);

/*!
 * \brief A function that returns the last element in the vector.
 *
 * \param v Reference to the vector object
 * 
 * \returns Pointer to the last element in the vector. NULL if the vector is
 * empty.
 */
VEC_API void *
vec_last(
    vec_t v);

/*!
 * \brief A function that returns the length of a vector
 *
 * \param v The vector object
 *
 * \returns Current length of the vector
 */
VEC_API size_t
vec_len(
  vec_t v);

/*!
 * \brief A function that returns the capacity of a vector
 *
 * \param v The vector object
 *
 * \returns Current capacity of the vector
 */
VEC_API size_t
vec_capacity(
  vec_t v);

/*!
 * \brief Calls the free operation (if exists) on every element, then sets
 * the length to 0.
 *
 * \param v The vector object
 *
 * \returns 0 on success
 */
VEC_API int
vec_clear(
  vec_t v);

/*!
 * \brief Sets the length of the vector to `len`.
 *
 * \details If `len` is less than `v`'s current length, then `v`'s length is
 * amended. Otherwise, the capacity is checked to make sure that there is enough
 * space for the new len.
 *
 * For `svec`, if the `len` is more than the already allocated capacity, it is
 * treated as an OOM.
 *
 * \param v Reference to the vector object
 * \param len The desired new length
 *
 * \returns `VEC_ERR_NONE` on success
 */
VEC_API vec_error_t
vec_setlen(
  vec_t *v, 
  size_t len);

/*!
 * \brief Sets the capacity of the vector to `cap`.
 *
 * \param v Reference to the vector object
 * \param cap The desired new capacity
 *
 * For stack-allocated vectors, `VEC_ERR_OOM` is returned
 *
 * \returns `VEC_ERR_NONE` on success, `VEC_ERR_OOM` on OOM
 */
VEC_API vec_error_t
vec_setcapacity(
  vec_t *v, 
  size_t cap);

/*!
 * \brief Grows the vector's capacity by a factor of `VEC_GROWTH_RATE`
 *
 * \param Reference to the vector object
 *
 * \returns `VEC_ERR_NONE` on success
 */
VEC_API vec_error_t
vec_grow(
  vec_t *v);

#ifdef VEC_IMPL
#undef VEC_IMPL

#ifdef VEC_API_CHECK
#define API_CHECK(x) do { x; } while(0)
#else
#define API_CHECK(x)
#endif

#include <stdlib.h>
#include <string.h>

#define vec_meta(v) \
  ((struct vec_meta_t *)v) - 1

#define __GET_METADATA__(v) \
  struct vec_meta_t *metadata = ((struct vec_meta_t *)(v)) - 1;

#define __SYNC_METADATA__(v) \
  metadata = ((struct vec_meta_t *)(v)) - 1;

vec_t
vec_init_impl(
    size_t elemsize, 
    elem_copy copy, 
    elem_destr destr)
{
  void *v = malloc(sizeof(struct vec_meta_t) + (VEC_INIT_CAP * elemsize));
  if (!v)
    return NULL;

  struct vec_meta_t *metadata = (struct vec_meta_t *)v;
  *metadata = (struct vec_meta_t){
    .length   = 0,
    .capacity = VEC_INIT_CAP,
    .elemsize = elemsize,
    .allocationType = VEC_ALLOCATION_TYPE_HEAP,
    .copy_fn  = copy,
    .destr_fn = destr,
  };

  return metadata + 1;
}

void *
vec_iter_begin(
    vec_t v)
{
  return v;
}

void *
vec_iter_end(
    vec_t v)
{
  __GET_METADATA__(v)

  return ((char *)v) + (metadata->elemsize * metadata->length);
}

void
vec_iter_next(
    vec_t v, 
    void **iter)
{
  __GET_METADATA__(v)
  *iter = ((char*)*iter) + metadata->elemsize;
}

void
vec_fini(
    vec_t v)
{
  __GET_METADATA__(v)

  if (metadata->destr_fn) {
    for (void *elem = vec_iter_begin(v); elem != vec_iter_end(v);
         vec_iter_next(v, &elem)) {
      metadata->destr_fn(elem);
    }
  }
  if(metadata->allocationType == VEC_ALLOCATION_TYPE_HEAP) {
    free(metadata);
  }
}

int
vec_push(
    vec_t *v, 
    void *val)
{
  __GET_METADATA__(*v)

  if (metadata->length == metadata->capacity) {
    vec_error_t grow_err = vec_grow(v);
    if(grow_err) {
      return grow_err;
    } else {
      __SYNC_METADATA__(*v)
    }
  }

  void *dst = ((char *)*v) + (metadata->length * metadata->elemsize);
  if (metadata->copy_fn) {
    metadata->copy_fn(dst, val);
  } else {
    memcpy(dst, val, metadata->elemsize);
  }

  return (int)metadata->length++;
}

VEC_API int
vec_append(
  vec_t *v,
  void **arr,
  size_t size)
{
  __GET_METADATA__(*v)
  size_t old_len = metadata->length;
  size_t req_len = old_len + size;

  vec_error_t setlen_err = vec_setlen(v, req_len);
  if(setlen_err) {
    return setlen_err;
  }
  __SYNC_METADATA__(*v)

  void *dst = ((char *)*v) + (old_len * metadata->elemsize);
  memcpy(dst, *arr, metadata->elemsize * size);

  return (int)old_len;
}

vec_error_t
vec_pop(
    vec_t *v, 
    void *out)
{
  __GET_METADATA__(*v)

  if(out != NULL) {
    void *src = ((char *)*v) + ((metadata->length-1) * metadata->elemsize);
    if (metadata->copy_fn) {
      metadata->copy_fn(out, src);
    } else {
      memcpy(out, src, metadata->elemsize);
    }
  } else {
    void *elem = ((char *)*v) + ((metadata->length-1) * metadata->elemsize);
    if (metadata->destr_fn) {
      metadata->destr_fn(elem);
    }
  }

  metadata->length--;

  return VEC_ERR_NONE;
}

void *
vec_last(
    vec_t v)
{
  __GET_METADATA__(v)

  if(metadata->length == 0) {
    return NULL;
  }

  return ((char *)v) + ((metadata->length-1) * metadata->elemsize);
}

size_t
vec_len(
    vec_t v)
{
  __GET_METADATA__(v)
  return metadata->length;
}

size_t
vec_capacity(
    vec_t v)
{
  __GET_METADATA__(v)
  return metadata->capacity;
}

int
vec_clear(
    vec_t v)
{
  __GET_METADATA__(v)

  if (metadata->destr_fn) {
    for (void *elem = vec_iter_begin(v); elem != vec_iter_end(v);
         vec_iter_next(v, &elem)) {
      metadata->destr_fn(elem);
    }
  }

  metadata->length = 0;
  return 0;
}

vec_error_t
vec_setlen(
    vec_t *v, 
    size_t len)
{
  __GET_METADATA__(*v)

  while(len > metadata->capacity) {
    vec_error_t grow_err = vec_grow(v);
    if(grow_err) {
      return grow_err;
    }
    __SYNC_METADATA__(*v)
  }

  metadata->length = len;
  return VEC_ERR_NONE;
}

vec_error_t
vec_setcapacity(
    vec_t *v, 
    size_t cap)
{
  __GET_METADATA__(*v)

  if(meta->allocationType == VEC_ALLOCATION_TYPE_STACK) {
    return VEC_ERR_OOM;
  }

  if(metadata->capacity == cap) {
    return VEC_ERR_NONE;
  }

  void *buf = ((char *)(*v) - sizeof(struct vec_meta_t));
  void *tmp = realloc(buf, sizeof(struct vec_meta_t) + (cap * metadata->elemsize));

  if (!tmp) {
    return VEC_ERR_OOM;
  }

  if(buf != tmp) {
    buf = tmp;
    metadata           = (struct vec_meta_t *)buf;
    *v = (char *)buf + sizeof(struct vec_meta_t);
  }

  metadata->capacity = cap;
  return VEC_ERR_NONE;
}

vec_error_t
vec_grow(
    vec_t *v)
{
  __GET_METADATA__(*v)
  return vec_setcapacity(v, metadata->capacity * VEC_GROWTH_RATE);
}

#endif

#endif
