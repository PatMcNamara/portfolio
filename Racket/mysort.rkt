;; The first three lines of this file were inserted by DrRacket. They record metadata
;; about the language level of this file in a form that our tools can easily process.
#reader(lib "htdp-advanced-reader.ss" "lang")((modname mysort) (read-case-sensitive #t) (teachpacks ()) (htdp-settings #(#t constructor repeating-decimal #t #t none #f () #f)))
; Patrick McNamara
; cs4250 hw5
; 01/12/2015
; Uses insertion sort to sort a list if numbers.

; inserts the atom value into the proper position in the sorted list rightlist. (start leftlist as '(). )
(define (insert value leftlist rightlist)
  (cond
    ((null? rightlist) (append leftlist (list value))) ; value is the largest element in the list, insert it 
    ((< value (car rightlist))
     (append leftlist (append (list value) rightlist))); value should go between the leftlist and the right list
    
    ; value must go farther to the right, take first element out of rightlist, put it at the end of left list and recursively call.
    (else (insert value
                  (append leftlist (list (car rightlist)))
                  (cdr rightlist))) 
  )
)

; sorts unsorted. (First call should have sorted be '() ).
(define (partsort unsorted sorted)
  (cond
    ((null? unsorted) sorted) ; If there are no more elements in unsorted, we are done.
    ; Recursive call. Take first element out of unsorted and insert it into proper position in sorted.
    (else (partsort (cdr unsorted) (insert (car unsorted) '() sorted))
    )
  )
)

; Helper function for partsort.
(define (mysort unsorted)
  (partsort unsorted '())
)