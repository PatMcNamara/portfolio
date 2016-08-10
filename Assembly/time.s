#----------------------------------------------------------------
#
# 	time.s
# 
#	Tells you the date in 'dd/mmm/yyyy' form, with the month
#	being the first 3 letter of the name of the month.  Will
#	be accurate until 12/31/2099.
#
#	Written by: Patrick McNamara	
#	Submited: 2/23/2007
#
#-----------------------------------------------------------------

	.equ	dev_STDOUT, 1
	.equ	sys_EXIT, 1
	.equ	sys_WRITE, 4
	.equ	sys_TIME, 13

	.section	.data
year:	.int	1970		# initialized to epoch
time:	.int	0		# system time
day:	.int	0

	.section	.text
_start:	# System call to get the current time
	movl	$sys_TIME, %eax
	xor	%ebx, %ebx
	int	$0x80

	sub	$28800, %eax	# minus 8 hours from time to account for time zones

	# 64-bit division to find number of days since epoch
	movl	$86400, %ebx	# divisor equals num of seconds in day
	xor	%edx, %edx	# zeros upper half of divisor
	divl	%ebx		# divide

	# 32-bit division to find number of years since epoch
	mov	$365, %ebx	# number of days in a year
	xor	%edx, %edx	# clears upper half of 32-bit divisor
	divw	%bx		# divide

	movw	%ax, %bx	# copy number of years
	add	year, %bx	# add to when epoch happened
	mov	%bx, year	# store sum as year

	# find number of days added because of leap year
	sub	$2, %ax		# first affecting leap year happened 2 years after epoch
	mov	$4, %bx		# prepair for integer division
	divb	%bl		# al is now number of leap years
	and	$0xFF, %ax	# masks remainder of division

	# what if number of days taken because of leap year cause negitive days?
	cmp	%ax, %dx	# check number of days vs number of leap days
	ja	month		# if carry and zero arn't set, result will be fine
	add	$365, %dx	# else, add a year to the day
	movw	year, %bx
	dec	%bx		# take out year that was added to the day
	mov	%bx, year
	

month:	# Find out what month it is
	sub	%ax, %dx	# take out number of leap days
	mov	%dx, day	# store differance as num of days

	# Prepair to find out what month it is
	mov	%edx, %eax	# moves num of days into eax
	xor	%ecx, %ecx	# clear count register
	mov	$1387, %ebx	# prepairs bit pattern 10101101011 (see below)
	call	isleap		# find out if it is a leap year
	jnz	nxmon		# if not, start the loop
	inc	%eax		# if yes, add 1 day (if before Feb 29th,
				#	will be taken off later)

	# loop to find how many months there are
nxmon:	mov	%eax, %edx	# save current month in edx
	inc	%ecx		# next month
	cmp	$2, %ecx	# check if month is February
	je	february

	# this part checks to see if there is 31 days in the month.
	# it does this by using the bit pattern established above,
	# this bit pattern is gotten by reversing the pattern you
	# would get if 1 represents months with 31 days and 0 represents
	# months with 30 days, while ignoring feb as this is handeled elsewhere
	ror	$1, %ebx	# sets cf to 1 if 31 days in month
	sbb	$0, %eax	# subtract cf from num of days 
	
not31:	sub	$30, %eax	# subtract 30 from num of days
	ja	nxmon		# if zf!=0 and cf!=0 loop again

endmon:	mov	%edx, %eax	# moves correct date before turning into a string

# we now have correct day, month and year

	.section	.data	
buff:	.ascii	"xx xxx xxxx \r\n"
lenght:	.int	. - buff
tmp:	.ascii	"xxxx"		#temporary storage area
montbl:	.ascii	" JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC" # month-table

	.section	.text
	# prints all the collected info
print:	# adds day of month to display buffer
	lea	buff, %edi
	call	eax2asc		# converts date in aex into ascii value for printing

	# adds month to display buffer
	mov	%ecx, %esi	# moves month into esi
	dec	%esi		# esi dec b/c since table values begin at 0 and not 1
	mov	montbl(, %esi, 4), %eax	# gets table value of month
	mov	%eax, buff+2	# adds month name to display buffer

	# adds year to display buffer
	mov	year, %eax	# moves year into eax
	lea	tmp, %edi	# loads temporary storage area
	call	eax2asc		# places ascii value of year in temp
	mov	tmp, %eax	# move year ascii value into a register
	mov	%eax, buff+7	# add year to the display buffer

	# prepair for write system call
	lea	buff, %ecx		# load display buffer
	mov	$sys_WRITE, %eax	# set system write call
	mov	$dev_STDOUT, %ebx	# set output device
	mov	lenght, %edx		# set lenght of display
	int	$0x80			# kernal interupt

	# exit program
	movl	$sys_EXIT, %eax	# set exit system call
	xor	%ebx, %ebx	# set exit code to 0
	int	$0x80		# kernal interupt

february: # lets february (and leap year) be properly accounted for
	sub	$28, %eax	# start by taking off 28 days
	jbe	endmon		# zf=0 or cf=0,  check if this month is feb
	call	isleap		# is it leap year?
	jnz	nxmon		# no, continue to next month
	dec	%eax		# yes, take off an extra day
	jz	endmon		# check again if it is feb
	jmp	nxmon		# procede to next month

isleap:	# function checks if this year is a leap year
	push	%eax
	push	%edx		# save registers
	push	%ebx
	mov	year, %eax	# set up for division...
	mov	$4, %ebx	# ... by 4
	xor	%edx, %edx	# clear upper half of divisor
	divl	%ebx		# divide
	cmp	$0, %edx	# set zf if remainder is 0
	pop	%ebx
	pop	%edx		# restore register
	pop	%eax
	ret			# return to caller

# All of this function, with a small exception (noted below) was
# taken from base10io.s, availible on the class website, and was
# written by Professor Cruse
#-----------------------------------------------------------------
eax2asc:	
#
# This procedure converts the 32-bit value found in register EAX 
# into its representation as a string of decimal numerals at EDI.
#
	pushal				# precerve CPU registers

	
	movl	$10, %ebx		# setup our divisor in EBX	
	xorl	%ecx, %ecx		# use ECX as digit counter
nxdiv:	# loop to generate and push the division-by-ten remainders 
	xorl	%edx, %edx		# extend dividend to qword
	divl	%ebx			# perform division by ten
	pushl	%edx			# save remainder on stack 
	incl	%ecx			# increment digit-counter 
	orl	%eax, %eax		# was the quotient zero?
	jnz	nxdiv			# no, do another division

	# added by myself
	cmp	$1, %ecx		# is there one diget in the number?
	jne	nxdgt			# no, continue normal before
	pushl	$0			# yes, push a zero to make the number 2 digit
	inc	%ecx			# and add 1 to the counter
	# end addition

nxdgt:	# loop to pop and store the digits in reversed order
	popl	%edx			# recover saved remainder
	addb	$'0', %dl		# convert number to ascii 
	movb	%dl, (%edi)		# place numeral in buffer
	incl	%edi			# and advance buffer-index
	loop	nxdgt			# convert another remainder

	popal				# restore saved registers
	ret				# return control to caller 
#-----------------------------------------------------------------

	.global	_start
	.end
