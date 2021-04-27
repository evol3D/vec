/*!
 * \file vec.h
 */
#ifndef EV_VEC_HEADER
#define EV_VEC_HEADER

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

typedef void *vec_t;

/*!
 * \brief For the sake of readability
 * \details Sample usage:
 * ```
 * vec(int) v = vec_init(int, 0, 0);
 * ```
 */
#define vec(T) T*

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
 * vec_init(int, 0, 0)
 * ```
 */
#define vec_init(type, copy, destr) vec_init_impl(sizeof(type), copy, destr)

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
 * 'OOM' issues, then the vector is left unchanged and a non-zero is returned.
 *
 * \param v Reference to the vector object
 * \param val A pointer to the element that is to be copied to the end of the
 * vector
 *
 * \returns An error code. If the operation was successful, then `0` is returned.
 */
VEC_API int
vec_push(
  vec_t *v, 
  void *val);

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
 * \param v Reference to the vector object
 * \param len The desired new length
 *
 * \returns 0 on success
 */
VEC_API int
vec_setlen(
  vec_t *v, 
  size_t len);

/*!
 * \brief Sets the capacity of the vector to `cap`.
 *
 * \param v Reference to the vector object
 * \param cap The desired new capacity
 *
 * \returns 0 on success
 */
VEC_API int
vec_setcapacity(
  vec_t *v, 
  size_t cap);

/*!
 * \brief Grows the vector's capacity by a factor of `VEC_GROWTH_RATE`
 *
 * \param Reference to the vector object
 *
 * \returns 0 on success
 */
VEC_API int
vec_grow(
  vec_t *v);

#ifdef EV_VEC_IMPL
#undef EV_VEC_IMPL

#include <stdlib.h>
#include <string.h>

//! Metadata that is stored with a vector. Unique to each vector.
struct vec_meta_t {
  //! The number of elements in the vector.
  size_t length;
  //! The maximum length of the vector before it needs to be resized.
  size_t capacity;
  //! The size (in bytes) of memory that each element takes.
  size_t elemsize;

  //! If set, the vector uses this function to copy new values into the vector.
  elem_copy copy_fn;
  //! If set, the vector uses this function to destroy stored values.
  elem_destr destr_fn;
};

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
  free(metadata);
}

int
vec_push(
    vec_t *v, 
    void *val)
{
  __GET_METADATA__(*v)

  if (metadata->length == metadata->capacity) {
    if(vec_grow(v)) {
      return 1;
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
  metadata->length++;
  return 0;
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
}

int
vec_setlen(
    vec_t *v, 
    size_t len)
{
  __GET_METADATA__(*v)

  while(len > metadata->capacity) {
    if(vec_grow(v)) {
      return 1;
    }
    __SYNC_METADATA__(*v)
  }

  metadata->length = len;
  return 0;
}

int
vec_setcapacity(
    vec_t *v, 
    size_t cap)
{
  __GET_METADATA__(*v)

  if(metadata->capacity == cap) {
    return 0;
  }

  void *buf = ((char *)(*v) - sizeof(struct vec_meta_t));
  void *tmp = realloc(buf, sizeof(struct vec_meta_t) + (cap * metadata->elemsize));

  if (!tmp) {
    return 1;
  }

  if(buf != tmp) {
    buf = tmp;
    metadata           = (struct vec_meta_t *)buf;
    *v = (char *)buf + sizeof(struct vec_meta_t);
  }

  metadata->capacity = cap;
  return 0;
}

int
vec_grow(
    vec_t *v)
{
  __GET_METADATA__(*v)
  return vec_setcapacity(v, metadata->capacity * VEC_GROWTH_RATE);
}

#endif

#endif
