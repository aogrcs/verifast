#include "putchar.h"

/*@
predicate_ctor close_putchar_pre(int queue_id, place t1, int c, place t2, list<int> buffer_contents)() =
  [1/2]ghost_cell<list<int> >(queue_id, buffer_contents)
  &*& token_without_invar(t1)
  &*& is_putchar_invar_updatable(?invar_updater, queue_id, t1, c, t2);
predicate_ctor close_putchar_post(int queue_id, place t1, int c, place t2, list<int> buffer_contents)() =
  [1/2]ghost_cell(queue_id, append(buffer_contents, {c}))
  &*& token_without_invar(t2);
@*/

/**
 * Places one integer in the given queue.
 * 
 * This is blocking. If the queue is full, it waits until it is not full anymore.
 */
void putchar(struct queue *queue, int x)
/*@ requires
  [?f_queue]queue(?queue_id, queue)
  &*& putchar_io(queue_id, ?t1, x, ?t2)
  &*& token(t1);
@*/
/*@ ensures
  token(t2)
  &*& [f_queue]queue(queue_id, queue);
@*/
{
  //@ open queue(_, _);
  //@ assert [f_queue]queue->mutex |-> ?mutex; // bind mutex so we know it won't change.
  mutex_acquire(queue->mutex);
  //@ open queue_invariant(queue_id, queue)();
   
  while (ring_buffer_is_full(queue->ring_buffer))
  /*@ invariant
    // from queue:
    [f_queue]queue-> mutex|-> mutex
    &*& [f_queue]queue->cond_can_push |-> ?cond_can_push
    &*& [f_queue]mutex_cond(cond_can_push, mutex)
  
    // from queue_invariant:
    &*& queue->ring_buffer |-> ?buffer
    &*& ring_buffer(buffer, _, ?buffer_contents)
    &*& [1/2]ghost_cell<list<int> >(queue_id, buffer_contents)
    
    &*& mutex_held(mutex, (queue_invariant)(queue_id, queue), currentThread, f_queue);
  @*/
  {
    //@ close queue_invariant(queue_id, queue)();
    mutex_cond_wait(queue->cond_can_push, queue->mutex);
    //@ open queue_invariant(queue_id, queue)();
  }
  
  bool was_empty = ring_buffer_is_empty(queue->ring_buffer);
  
  ring_buffer_push(queue->ring_buffer, x);
  
  if (was_empty){
    mutex_cond_signal(queue->cond_can_pop);
  }
  //@ open putchar_io(queue_id, t1, x, t2);
  
  //@ predicate() pre  = close_putchar_pre( queue_id, t1, x, t2, buffer_contents);
  //@ predicate() post = close_putchar_post(queue_id, t1, x, t2, buffer_contents);
  //@ close close_putchar_pre(queue_id, t1, x, t2, buffer_contents)();
  /*@
  produce_lemma_function_pointer_chunk(empty_lemma) : ghost_mutex_critical_section_t(place_io_invar(t1), pre, post)()
  {
    open close_putchar_pre( queue_id, t1, x, t2, buffer_contents)();
    assert is_putchar_invar_updatable(?invar_updater, queue_id, t1, x, t2);
    close exists(place_io_invar(t1));
    open token_without_invar(t1);
    invar_updater();
    close token_without_invar(t2);
    close close_putchar_post(queue_id, t1, x, t2, buffer_contents)();
    leak is_putchar_invar_updatable(_, _, _, _, _);
    call();
  }
  {
    ghost_mutex_use(place_mutex(t1), pre, post);
  }
  @*/
  //@ open close_putchar_post(queue_id, t1, x, t2, buffer_contents)();
  //@ close queue_invariant(queue_id, queue)();
  mutex_release(queue->mutex);
  //@ close [f_queue]queue(queue_id, queue);
}
