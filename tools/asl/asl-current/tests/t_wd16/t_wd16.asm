	cpu	wd16
	page	0

	nop				; 0x0000
	reset				; 0x0001
	ien				; 0x0002
	ids				; 0x0003
	halt				; 0x0004
	xct				; 0x0005
	bpt				; 0x0006
	wfi				; 0x0007
	rsvc				; 0x0008
	rrtt				; 0x0009
	;save				; 0x000a
	savs				; 0x000b
	rest				; 0x000c
	rrtn				; 0x000d
	rsts				; 0x000e
	rtt				; 0x000f

	iak	r5			; 0x0015
	rtn	r6			; 0x001e
	msko	r2			; 0x0022
	prtn	r4			; 0x002c

	lcc	13			; 0x003d

	svca	0x12			; 0x0052
	svcb	0x23			; 0x00a3
	svcc	0x34			; 0x00f4

	br	*+10			; 0x0104
	bne	*-20			; 0x02f5
	beq	*+30			; 0x030e
	bge	*-40			; 0x04eb
	blt	*+50			; 0x0518
	bgt	*-50			; 0x06e6
	ble	*+70			; 0x0722
	bpl	*-50			; 0x80e6
	bmi	*+90			; 0x812c
	bhi	*-40			; 0x82eb
	blos	*+110			; 0x8336
	bvc	*-30			; 0x84f0
	bvs	*+130			; 0x8540
	bcc	*-20			; 0x86f5
	bhis	*+150			; 0x864a
	bcs	*-10			; 0x87fa
	blo	*+170			; 0x8754

	ror	r1			; 0x0a01
	ror	@r2			; 0x0a0a
	ror	(r3)+			; 0x0a13
	ror	@(r4)+			; 0x0a1c
	ror	-(r5)			; 0x0a25
	ror	@-(r6)			; 0x0a2e
	ror	0x1234(r1)		; 0x0a31 0x1234
	ror	@0x1234(r2)		; 0x0a3a 0x1234
	expect	1350
	ror	#0123456
	endexpect
	ror	@#0x1234		; 0x0a1f 0x1234
	ror	0x1234			; 0x0a37 0x11c8
	ror	@0x1234			; 0x0a3f 0x11c4

	rol	r1			; 0x0a41
	rol	@r2			; 0x0a4a
	rol	(r3)+			; 0x0a53
	rol	@(r4)+			; 0x0a5c
	rol	-(r5)			; 0x0a65
	rol	@-(r6)			; 0x0a6e
	rol	0x1234(r1)		; 0x0a71 0x1234
	rol	@0x1234(r2)		; 0x0a7a 0x1234
	expect	1350
	rol	#0123456
	endexpect
	rol	@#0x1234		; 0x0a5f 0x1234
	rol	0x1234			; 0x0a77 0x11a8
	rol	@0x1234			; 0x0a7f 0x11a4

	tst	r1			; 0x0a81
	tst	@r2			; 0x0a8a
	tst	(r3)+			; 0x0a93
	tst	@(r4)+			; 0x0a9c
	tst	-(r5)			; 0x0aa5
	tst	@-(r6)			; 0x0aae
	tst	0x1234(r1)		; 0x0ab1 0x1234
	tst	@0x1234(r2)		; 0x0aba 0x1234
	expect	1350
	tst	#0123456
	endexpect
	tst	@#0x1234		; 0x0a9f 0x1234
	tst	0x1234			; 0x0ab7 0x1188
	tst	@0x1234			; 0x0abf 0x1184

	asl	r1			; 0x0ac1
	asl	@r2			; 0x0aca
	asl	(r3)+			; 0x0ad3
	asl	@(r4)+			; 0x0adc
	asl	-(r5)			; 0x0ae5
	asl	@-(r6)			; 0x0aee
	asl	0x1234(r1)		; 0x0af1 0x1234
	asl	@0x1234(r2)		; 0x0afa 0x1234
	expect	1350
	asl	#0123456
	endexpect
	asl	@#0x1234		; 0x0adf 0x1234
	asl	0x1234			; 0x0af7 0x1168
	asl	@0x1234			; 0x0aff 0x1164

	set	r1			; 0x0b01
	set	@r2			; 0x0b0a
	set	(r3)+			; 0x0b13
	set	@(r4)+			; 0x0b1c
	set	-(r5)			; 0x0b25
	set	@-(r6)			; 0x0b2e
	set	0x1234(r1)		; 0x0b31 0x1234
	set	@0x1234(r2)		; 0x0b3a 0x1234
	expect	1350
	set	#0123456
	endexpect
	set	@#0x1234		; 0x0b1f 0x1234
	set	0x1234			; 0x0b37 0x1148
	set	@0x1234			; 0x0b3f 0x1144

	clr	r1			; 0x0b41
	clr	@r2			; 0x0b4a
	clr	(r3)+			; 0x0b53
	clr	@(r4)+			; 0x0b5c
	clr	-(r5)			; 0x0b65
	clr	@-(r6)			; 0x0b6e
	clr	0x1234(r1)		; 0x0b71 0x1234
	clr	@0x1234(r2)		; 0x0b7a 0x1234
	expect	1350
	clr	#0123456
	endexpect
	clr	@#0x1234		; 0x0b5f 0x1234
	clr	0x1234			; 0x0b77 0x1128
	clr	@0x1234			; 0x0b7f 0x1124

	asr	r1			; 0x0b81
	asr	@r2			; 0x0b8a
	asr	(r3)+			; 0x0b93
	asr	@(r4)+			; 0x0b9c
	asr	-(r5)			; 0x0ba5
	asr	@-(r6)			; 0x0bae
	asr	0x1234(r1)		; 0x0bb1 0x1234
	asr	@0x1234(r2)		; 0x0bba 0x1234
	expect	1350
	asr	#0123456
	endexpect
	asr	@#0x1234		; 0x0b9f 0x1234
	asr	0x1234			; 0x0bb7 0x1108
	asr	@0x1234			; 0x0bbf 0x1104

	swab	r1			; 0x0bc1
	swab	@r2			; 0x0bca
	swab	(r3)+			; 0x0bd3
	swab	@(r4)+			; 0x0bdc
	swab	-(r5)			; 0x0be5
	swab	@-(r6)			; 0x0bee
	swab	0x1234(r1)		; 0x0bf1 0x1234
	swab	@0x1234(r2)		; 0x0bfa 0x1234
	expect	1350
	swab	#0123456
	endexpect
	swab	@#0x1234		; 0x0bdf 0x1234
	swab	0x1234			; 0x0bf7 0x10e8
	swab	@0x1234			; 0x0bff 0x10e4

	com	r1			; 0x0c01
	com	@r2			; 0x0c0a
	com	(r3)+			; 0x0c13
	com	@(r4)+			; 0x0c1c
	com	-(r5)			; 0x0c25
	com	@-(r6)			; 0x0c2e
	com	0x1234(r1)		; 0x0c31 0x1234
	com	@0x1234(r2)		; 0x0c3a 0x1234
	expect	1350
	com	#0123456
	endexpect
	com	@#0x1234		; 0x0c1f 0x1234
	com	0x1234			; 0x0c37 0x10c8
	com	@0x1234			; 0x0c3f 0x10c4

	neg	r1			; 0x0c41
	neg	@r2			; 0x0c4a
	neg	(r3)+			; 0x0c53
	neg	@(r4)+			; 0x0c5c
	neg	-(r5)			; 0x0c65
	neg	@-(r6)			; 0x0c6e
	neg	0x1234(r1)		; 0x0c71 0x1234
	neg	@0x1234(r2)		; 0x0c7a 0x1234
	expect	1350
	neg	#0123456
	endexpect
	neg	@#0x1234		; 0x0c5f 0x1234
	neg	0x1234			; 0x0c77 0x10a8
	neg	@0x1234			; 0x0c7f 0x10a4

	inc	r1			; 0x0c81
	inc	@r2			; 0x0c8a
	inc	(r3)+			; 0x0c93
	inc	@(r4)+			; 0x0c9c
	inc	-(r5)			; 0x0ca5
	inc	@-(r6)			; 0x0cae
	inc	0x1234(r1)		; 0x0cb1 0x1234
	inc	@0x1234(r2)		; 0x0cba 0x1234
	expect	1350
	inc	#0123456
	endexpect
	inc	@#0x1234		; 0x0c9f 0x1234
	inc	0x1234			; 0x0cb7 0x1088
	inc	@0x1234			; 0x0cbf 0x1084

	dec	r1			; 0x0cc1
	dec	@r2			; 0x0cca
	dec	(r3)+			; 0x0cd3
	dec	@(r4)+			; 0x0cdc
	dec	-(r5)			; 0x0ce5
	dec	@-(r6)			; 0x0cee
	dec	0x1234(r1)		; 0x0cf1 0x1234
	dec	@0x1234(r2)		; 0x0cfa 0x1234
	expect	1350
	dec	#0123456
	endexpect
	dec	@#0x1234		; 0x0cdf 0x1234
	dec	0x1234			; 0x0cf7 0x1068
	dec	@0x1234			; 0x0cff 0x1064

	iw2	r1			; 0x0d01
	iw2	@r2			; 0x0d0a
	iw2	(r3)+			; 0x0d13
	iw2	@(r4)+			; 0x0d1c
	iw2	-(r5)			; 0x0d25
	iw2	@-(r6)			; 0x0d2e
	iw2	0x1234(r1)		; 0x0d31 0x1234
	iw2	@0x1234(r2)		; 0x0d3a 0x1234
	expect	1350
	iw2	#0123456
	endexpect
	iw2	@#0x1234		; 0x0d1f 0x1234
	iw2	0x1234			; 0x0d37 0x1048
	iw2	@0x1234			; 0x0d3f 0x1044

	sxt	r1			; 0x0d41
	sxt	@r2			; 0x0d4a
	sxt	(r3)+			; 0x0d53
	sxt	@(r4)+			; 0x0d5c
	sxt	-(r5)			; 0x0d65
	sxt	@-(r6)			; 0x0d6e
	sxt	0x1234(r1)		; 0x0d71 0x1234
	sxt	@0x1234(r2)		; 0x0d7a 0x1234
	expect	1350
	sxt	#0123456
	endexpect
	sxt	@#0x1234		; 0x0d5f 0x1234
	sxt	0x1234			; 0x0d77 0x1028
	sxt	@0x1234			; 0x0d7f 0x1024

	tcall	r1			; 0x0d81
	tcall	@r2			; 0x0d8a
	tcall	(r3)+			; 0x0d93
	tcall	@(r4)+			; 0x0d9c
	tcall	-(r5)			; 0x0da5
	tcall	@-(r6)			; 0x0dae
	tcall	0x1234(r1)		; 0x0db1 0x1234
	tcall	@0x1234(r2)		; 0x0dba 0x1234
	tcall	#0123456		; 0x0d97 0xa72e
	tcall	@#0x1234		; 0x0d9f 0x1234
	tcall	0x1234			; 0x0db7 0x1004
	tcall	@0x1234			; 0x0dbf 0x1000

	tjmp	r1			; 0x0dc1
	tjmp	@r2			; 0x0dca
	tjmp	(r3)+			; 0x0dd3
	tjmp	@(r4)+			; 0x0ddc
	tjmp	-(r5)			; 0x0de5
	tjmp	@-(r6)			; 0x0dee
	tjmp	0x1234(r1)		; 0x0df1 0x1234
	tjmp	@0x1234(r2)		; 0x0dfa 0x1234
	tjmp	#0123456		; 0x0dd7 0xa72e
	tjmp	@#0x1234		; 0x0ddf 0x1234
	tjmp	0x1234			; 0x0df7 0x0fe0
	tjmp	@0x1234			; 0x0dff 0x0fdc

	rorb	r1			; 0x8a01
	rorb	@r2			; 0x8a0a
	rorb	(r3)+			; 0x8a13
	rorb	@(r4)+			; 0x8a1c
	rorb	-(r5)			; 0x8a25
	rorb	@-(r6)			; 0x8a2e
	rorb	0x1234(r1)		; 0x8a31 0x1234
	rorb	@0x1234(r2)		; 0x8a3a 0x1234
	expect	1350
	rorb	#0123456
	endexpect
	rorb	@#0x1234		; 0x8a1f 0x1234
	rorb	0x1234			; 0x8a37 0x0fc0
	rorb	@0x1234			; 0x8a3f 0x0fbc

	rolb	r1			; 0x8a41
	rolb	@r2			; 0x8a4a
	rolb	(r3)+			; 0x8a53
	rolb	@(r4)+			; 0x8a5c
	rolb	-(r5)			; 0x8a65
	rolb	@-(r6)			; 0x8a6e
	rolb	0x1234(r1)		; 0x8a71 0x1234
	rolb	@0x1234(r2)		; 0x8a7a 0x1234
	expect	1350
	rolb	#0123456
	endexpect
	rolb	@#0x1234		; 0x8a5f 0x1234
	rolb	0x1234			; 0x8a77 0x0fa0
	rolb	@0x1234			; 0x8a7f 0x0f9c

	tstb	r1			; 0x8a81
	tstb	@r2			; 0x8a8a
	tstb	(r3)+			; 0x8a93
	tstb	@(r4)+			; 0x8a9c
	tstb	-(r5)			; 0x8aa5
	tstb	@-(r6)			; 0x8aae
	tstb	0x1234(r1)		; 0x8ab1 0x1234
	tstb	@0x1234(r2)		; 0x8aba 0x1234
	expect	1350
	tstb	#0123456
	endexpect
	tstb	@#0x1234		; 0x8a9f 0x1234
	tstb	0x1234			; 0x8ab7 0x0f80
	tstb	@0x1234			; 0x8abf 0x0f7c

	aslb	r1			; 0x8ac1
	aslb	@r2			; 0x8aca
	aslb	(r3)+			; 0x8ad3
	aslb	@(r4)+			; 0x8adc
	aslb	-(r5)			; 0x8ae5
	aslb	@-(r6)			; 0x8aee
	aslb	0x1234(r1)		; 0x8af1 0x1234
	aslb	@0x1234(r2)		; 0x8afa 0x1234
	expect	1350
	aslb	#0123456
	endexpect
	aslb	@#0x1234		; 0x8adf 0x1234
	aslb	0x1234			; 0x8af7 0x0f60
	aslb	@0x1234			; 0x8aff 0x0f5c

	setb	r1			; 0x8b01
	setb	@r2			; 0x8b0a
	setb	(r3)+			; 0x8b13
	setb	@(r4)+			; 0x8b1c
	setb	-(r5)			; 0x8b25
	setb	@-(r6)			; 0x8b2e
	setb	0x1234(r1)		; 0x8b31 0x1234
	setb	@0x1234(r2)		; 0x8b3a 0x1234
	expect	1350
	setb	#0123456
	endexpect
	setb	@#0x1234		; 0x8b1f 0x1234
	setb	0x1234			; 0x8b37 0x0f40
	setb	@0x1234			; 0x8b3f 0x0f3c

	clrb	r1			; 0x8b41
	clrb	@r2			; 0x8b4a
	clrb	(r3)+			; 0x8b53
	clrb	@(r4)+			; 0x8b5c
	clrb	-(r5)			; 0x8b65
	clrb	@-(r6)			; 0x8b6e
	clrb	0x1234(r1)		; 0x8b71 0x1234
	clrb	@0x1234(r2)		; 0x8b7a 0x1234
	expect	1350
	clrb	#0123456
	endexpect
	clrb	@#0x1234		; 0x8b5f 0x1234
	clrb	0x1234			; 0x8b77 0x0f20
	clrb	@0x1234			; 0x8b7f 0x0f1c

	asrb	r1			; 0x8b81
	asrb	@r2			; 0x8b8a
	asrb	(r3)+			; 0x8b93
	asrb	@(r4)+			; 0x8b9c
	asrb	-(r5)			; 0x8ba5
	asrb	@-(r6)			; 0x8bae
	asrb	0x1234(r1)		; 0x8bb1 0x1234
	asrb	@0x1234(r2)		; 0x8bba 0x1234
	expect	1350
	asrb	#0123456
	endexpect
	asrb	@#0x1234		; 0x8b9f 0x1234
	asrb	0x1234			; 0x8bb7 0x0f00
	asrb	@0x1234			; 0x8bbf 0x0efc

	swad	r1			; 0x8bc1
	swad	@r2			; 0x8bca
	swad	(r3)+			; 0x8bd3
	swad	@(r4)+			; 0x8bdc
	swad	-(r5)			; 0x8be5
	swad	@-(r6)			; 0x8bee
	swad	0x1234(r1)		; 0x8bf1 0x1234
	swad	@0x1234(r2)		; 0x8bfa 0x1234
	expect	1350
	swad	#0123456
	endexpect
	swad	@#0x1234		; 0x8bdf 0x1234
	swad	0x1234			; 0x8bf7 0x0ee0
	swad	@0x1234			; 0x8bff 0x0edc

	comb	r1			; 0x8c01
	comb	@r2			; 0x8c0a
	comb	(r3)+			; 0x8c13
	comb	@(r4)+			; 0x8c1c
	comb	-(r5)			; 0x8c25
	comb	@-(r6)			; 0x8c2e
	comb	0x1234(r1)		; 0x8c31 0x1234
	comb	@0x1234(r2)		; 0x8c3a 0x1234
	expect	1350
	comb	#0123456
	endexpect
	comb	@#0x1234		; 0x8c1f 0x1234
	comb	0x1234			; 0x8c37 0x0ec0
	comb	@0x1234			; 0x8c3f 0x0ebc

	negb	r1			; 0x8c41
	negb	@r2			; 0x8c4a
	negb	(r3)+			; 0x8c53
	negb	@(r4)+			; 0x8c5c
	negb	-(r5)			; 0x8c65
	negb	@-(r6)			; 0x8c6e
	negb	0x1234(r1)		; 0x8c71 0x1234
	negb	@0x1234(r2)		; 0x8c7a 0x1234
	expect	1350
	negb	#0123456
	endexpect
	negb	@#0x1234		; 0x8c5f 0x1234
	negb	0x1234			; 0x8c77 0x0ea0
	negb	@0x1234			; 0x8c7f 0x0e9c

	incb	r1			; 0x8c81
	incb	@r2			; 0x8c8a
	incb	(r3)+			; 0x8c93
	incb	@(r4)+			; 0x8c9c
	incb	-(r5)			; 0x8ca5
	incb	@-(r6)			; 0x8cae
	incb	0x1234(r1)		; 0x8cb1 0x1234
	incb	@0x1234(r2)		; 0x8cba 0x1234
	expect	1350
	incb	#0123456
	endexpect
	incb	@#0x1234		; 0x8c9f 0x1234
	incb	0x1234			; 0x8cb7 0x0e80
	incb	@0x1234			; 0x8cbf 0x0e7c

	decb	r1			; 0x8cc1
	decb	@r2			; 0x8cca
	decb	(r3)+			; 0x8cd3
	decb	@(r4)+			; 0x8cdc
	decb	-(r5)			; 0x8ce5
	decb	@-(r6)			; 0x8cee
	decb	0x1234(r1)		; 0x8cf1 0x1234
	decb	@0x1234(r2)		; 0x8cfa 0x1234
	expect	1350
	decb	#0123456
	endexpect
	decb	@#0x1234		; 0x8cdf 0x1234
	decb	0x1234			; 0x8cf7 0x0e60
	decb	@0x1234			; 0x8cff 0x0e5c

	lsts	r1			; 0x8d01
	lsts	@r2			; 0x8d0a
	lsts	(r3)+			; 0x8d13
	lsts	@(r4)+			; 0x8d1c
	lsts	-(r5)			; 0x8d25
	lsts	@-(r6)			; 0x8d2e
	lsts	0x1234(r1)		; 0x8d31 0x1234
	lsts	@0x1234(r2)		; 0x8d3a 0x1234
	lsts	#0123456		; 0x8d17 0xa72e
	lsts	@#0x1234		; 0x8d1f 0x1234
	lsts	0x1234			; 0x8d37 0x0e3c
	lsts	@0x1234			; 0x8d3f 0x0e38

	ssts	r1			; 0x8d41
	ssts	@r2			; 0x8d4a
	ssts	(r3)+			; 0x8d53
	ssts	@(r4)+			; 0x8d5c
	ssts	-(r5)			; 0x8d65
	ssts	@-(r6)			; 0x8d6e
	ssts	0x1234(r1)		; 0x8d71 0x1234
	ssts	@0x1234(r2)		; 0x8d7a 0x1234
	expect	1350
	ssts	#0123456
	endexpect
	ssts	@#0x1234		; 0x8d5f 0x1234
	ssts	0x1234			; 0x8d77 0x0e1c
	ssts	@0x1234			; 0x8d7f 0x0e18

	adc	r1			; 0x8d81
	adc	@r2			; 0x8d8a
	adc	(r3)+			; 0x8d93
	adc	@(r4)+			; 0x8d9c
	adc	-(r5)			; 0x8da5
	adc	@-(r6)			; 0x8dae
	adc	0x1234(r1)		; 0x8db1 0x1234
	adc	@0x1234(r2)		; 0x8dba 0x1234
	expect	1350
	adc	#0123456
	endexpect
	adc	@#0x1234		; 0x8d9f 0x1234
	adc	0x1234			; 0x8db7 0x0dfc
	adc	@0x1234			; 0x8dbf 0x0df8

	sbc	r1			; 0x8dc1
	sbc	@r2			; 0x8dca
	sbc	(r3)+			; 0x8dd3
	sbc	@(r4)+			; 0x8ddc
	sbc	-(r5)			; 0x8de5
	sbc	@-(r6)			; 0x8dee
	sbc	0x1234(r1)		; 0x8df1 0x1234
	sbc	@0x1234(r2)		; 0x8dfa 0x1234
	expect	1350
	sbc	#0123456
	endexpect
	sbc	@#0x1234		; 0x8ddf 0x1234
	sbc	0x1234			; 0x8df7 0x0ddc
	sbc	@0x1234			; 0x8dff 0x0dd8

	expect	1315
	addi	0,r1
	endexpect
	addi	1,r1			; 0x0840
	addi	16,r1			; 0x084f
	expect	1320
	addi	17,r1
	endexpect

	expect	1315
	subi	0,r1
	endexpect
	subi	1,r1			; 0x0850
	subi	16,r1			; 0x085f
	expect	1320
	subi	17,r1
	endexpect

	expect	1315
	bici	0,r1
	endexpect
	bici	1,r1			; 0x0860
	bici	16,r1			; 0x086f
	expect	1320
	bici	17,r1
	endexpect

	expect	1315
	movi	0,r1
	endexpect
	movi	1,r1			; 0x0870
	movi	16,r1			; 0x087f
	expect	1320
	movi	17,r1
	endexpect

	expect	1315
	ssrr	0,r1
	endexpect
	ssrr	1,r1			; 0x8840
	ssrr	16,r1			; 0x884f
	expect	1320
	ssrr	17,r1
	endexpect

	expect	1315
	sslr	0,r1
	endexpect
	sslr	1,r1			; 0x8850
	sslr	16,r1			; 0x885f
	expect	1320
	sslr	17,r1
	endexpect

	expect	1315
	ssra	0,r1
	endexpect
	ssra	1,r1			; 0x8860
	ssra	16,r1			; 0x886f
	expect	1320
	ssra	17,r1
	endexpect

	expect	1315
	ssla	0,r1
	endexpect
	ssla	1,r1			; 0x8870
	ssla	16,r1			; 0x887f
	expect	1320
	ssla	17,r1
	endexpect

	expect	1315
	sdrr	0,r1
	endexpect
	sdrr	1,r1			; 0x8e40
	sdrr	16,r1			; 0x8e4f
	expect	1320
	sdrr	17,r1
	endexpect

	expect	1315
	sdlr	0,r1
	endexpect
	sdlr	1,r1			; 0x8e50
	sdlr	16,r1			; 0x8e5f
	expect	1320
	sdlr	17,r1
	endexpect

	expect	1315
	sdra	0,r1
	endexpect
	sdra	1,r1			; 0x8e60
	sdra	16,r1			; 0x8e6f
	expect	1320
	sdra	17,r1
	endexpect

	expect	1315
	sdla	0,r1
	endexpect
	sdla	1,r1			; 0x8e70
	sdla	16,r1			; 0x8e7f
	expect	1320
	sdla	17,r1
	endexpect

	mbwu	r4,r5			; 0x0e25
	mbwd	r4,r5			; 0x0e65
	mbbu	r4,r5			; 0x0ea5
	mbbd	r4,r5			; 0x0ee5
	mbwa	r4,r5			; 0x0f25
	mbba	r4,r5			; 0x0f65
	mabw	r4,r5			; 0x0fa5
	mabb	r4,r5			; 0x0fe5

	expect	1350
	jsr	r6,r1
	endexpect
	jsr	r6,@r2			; 0x718a
	jsr	r6,(r3)+		; 0x7193
	jsr	r6,@(r4)+		; 0x719c
	jsr	r6,-(r5)		; 0x71a5
	jsr	r6,@-(r6)		; 0x71ae
	jsr	r6,0x1234(r1)		; 0x71b1 0x1234
	jsr	r6,@0x1234(r2)		; 0x71ba 0x1234
	jsr	r6,#0123456		; 0x7197 0xa72e
	jsr	r6,@#0x1234		; 0x719f 0x1234
	jsr	r6,0x1234		; 0x71b7 0x0d7a
	jsr	r6,@0x1234		; 0x71bf 0x0d76

	; CALL dst is an alias for JSR PC,dst

	expect	1350
	call	r1
	endexpect
	call	@r2			; 0x71ca
	call	(r3)+			; 0x71d3
	call	@(r4)+			; 0x71dc
	call	-(r5)			; 0x71e5
	call	@-(r6)			; 0x71ee
	call	0x1234(r1)		; 0x71f1 0x1234
	call	@0x1234(r2)		; 0x71fa 0x1234
	call	#0123456		; 0x71d7 0xa72e
	call	@#0x1234		; 0x71df 0x1234
	call	0x1234			; 0x71f7 0x0d58
	call	@0x1234			; 0x71ff 0x0d54

	expect	1350
	lea	r6,r1
	endexpect
	lea	r6,@r2			; 0x738a
	lea	r6,(r3)+		; 0x7393
	lea	r6,@(r4)+		; 0x739c
	lea	r6,-(r5)		; 0x73a5
	lea	r6,@-(r6)		; 0x73ae
	lea	r6,0x1234(r1)		; 0x73b1 0x1234
	lea	r6,@0x1234(r2)		; 0x73ba 0x1234
	lea	r6,#0123456		; 0x7397 0xa72e
	lea	r6,@#0x1234		; 0x739f 0x1234
	lea	r6,0x1234		; 0x73b7 0x0d36
	lea	r6,@0x1234		; 0x73bf 0x0d32

	; JMP dst is an alias for LEA PC,dst

	expect	1350
	jmp	r1
	endexpect
	jmp	@r2			; 0x73ca
	jmp	(r3)+			; 0x73d3
	jmp	@(r4)+			; 0x73dc
	jmp	-(r5)			; 0x73e5
	jmp	@-(r6)			; 0x73ee
	jmp	0x1234(r1)		; 0x73f1 0x1234
	jmp	@0x1234(r2)		; 0x73fa 0x1234
	jmp	#0123456		; 0x73d7 0xa72e
	jmp	@#0x1234		; 0x73df 0x1234
	jmp	0x1234			; 0x73f7 0x0d14
	jmp	@0x1234			; 0x73ff 0x0d10

	ash	r6,r1			; 0x7581
	ash	r6,@r2			; 0x758a
	ash	r6,(r3)+		; 0x7593
	ash	r6,@(r4)+		; 0x759c
	ash	r6,-(r5)		; 0x75a5
	ash	r6,@-(r6)		; 0x75ae
	ash	r6,0x1234(r1)		; 0x75b1 0x1234
	ash	r6,@0x1234(r2)		; 0x75ba 0x1234
	ash	r6,#0123456		; 0x7597 0xa72e
	ash	r6,@#0x1234		; 0x759f 0x1234
	ash	r6,0x1234		; 0x75b7 0x0cf0
	ash	r6,@0x1234		; 0x75bf 0x0cec

	xch	r6,r1			; 0x7981
	xch	r6,@r2			; 0x798a
	xch	r6,(r3)+		; 0x7993
	xch	r6,@(r4)+		; 0x799c
	xch	r6,-(r5)		; 0x79a5
	xch	r6,@-(r6)		; 0x79ae
	xch	r6,0x1234(r1)		; 0x79b1 0x1234
	xch	r6,@0x1234(r2)		; 0x79ba 0x1234
	xch	r6,#0123456		; 0x7997 0xa72e
	xch	r6,@#0x1234		; 0x799f 0x1234
	xch	r6,0x1234		; 0x79b7 0x0ccc
	xch	r6,@0x1234		; 0x79bf 0x0cc8

	ashc	r6,r1			; 0x7b81
	ashc	r6,@r2			; 0x7b8a
	ashc	r6,(r3)+		; 0x7b93
	ashc	r6,@(r4)+		; 0x7b9c
	ashc	r6,-(r5)		; 0x7ba5
	ashc	r6,@-(r6)		; 0x7bae
	ashc	r6,0x1234(r1)		; 0x7bb1 0x1234
	ashc	r6,@0x1234(r2)		; 0x7bba 0x1234
	ashc	r6,#0123456		; 0x7b97 0xa72e
	ashc	r6,@#0x1234		; 0x7b9f 0x1234
	ashc	r6,0x1234		; 0x7bb7 0x0ca8
	ashc	r6,@0x1234		; 0x7bbf 0x0ca4

	mul	r6,r1			; 0x7d81
	mul	r6,@r2			; 0x7d8a
	mul	r6,(r3)+		; 0x7d93
	mul	r6,@(r4)+		; 0x7d9c
	mul	r6,-(r5)		; 0x7da5
	mul	r6,@-(r6)		; 0x7dae
	mul	r6,0x1234(r1)		; 0x7db1 0x1234
	mul	r6,@0x1234(r2)		; 0x7dba 0x1234
	mul	r6,#0123456		; 0x7d97 0xa72e
	mul	r6,@#0x1234		; 0x7d9f 0x1234
	mul	r6,0x1234		; 0x7db7 0x0c84
	mul	r6,@0x1234		; 0x7dbf 0x0c80

	div	r6,r1			; 0x7f81
	div	r6,@r2			; 0x7f8a
	div	r6,(r3)+		; 0x7f93
	div	r6,@(r4)+		; 0x7f9c
	div	r6,-(r5)		; 0x7fa5
	div	r6,@-(r6)		; 0x7fae
	div	r6,0x1234(r1)		; 0x7fb1 0x1234
	div	r6,@0x1234(r2)		; 0x7fba 0x1234
	div	r6,#0123456		; 0x7f97 0xa72e
	div	r6,@#0x1234		; 0x7f9f 0x1234
	div	r6,0x1234		; 0x7fb7 0x0c60
	div	r6,@0x1234		; 0x7fbf 0x0c5c

	expect	1370
	sob	r6,*-126
	endexpect
	sob	r6,*-124
	sob	r6,*+2
	expect	1370
	sob	r6,*+4
	endexpect

	add	r1,r2			; 0x1042
	add	@r2,@r3			; 0x128b
	add	(r3)+,(r4)+		; 0x14d4
	add	@(r4)+,@(r5)+		; 0x171d
	add	-(r5),-(r6)		; 0x1966
	add	@-(r6),@-(r1)		; 0x1bA9
	add	0x1234(r1),0x1345(r2)	; 0x1c72 0x1234 0x1345
	add	@0x1234(r2),@0x1345(r3)	; 0x1ebb 0x1234 0x1345
	expect	1350
	add	#0x1234,#0x1345
	endexpect
	add	#0x1234,r4		; 0x15c4 0x1234
	add	@#0x1234,@#0x1345	; 0x17df 0x1234 0x1345
	add	0x1234,0x1345		; 0x1df7 0x0c32 0x0d41
	add	@0x1234,@0x1345		; 0x1fff 0x0c2c 0x0d3B

	sub	r1,r2			; 0x2042
	sub	@r2,@r3			; 0x228b
	sub	(r3)+,(r4)+		; 0x24d4
	sub	@(r4)+,@(r5)+		; 0x271d
	sub	-(r5),-(r6)		; 0x2966
	sub	@-(r6),@-(r1)		; 0x2bA9
	sub	0x1234(r1),0x1345(r2)	; 0x2c72 0x1234 0x1345
	sub	@0x1234(r2),@0x1345(r3)	; 0x2ebb 0x1234 0x1345
	expect	1350
	sub	#0x1234,#0x1345
	endexpect
	sub	#0x1234,r4		; 0x25c4 0x1234
	sub	@#0x1234,@#0x1345	; 0x27df 0x1234 0x1345
	sub	0x1234,0x1345		; 0x2df7 0x0c04 0x0d13
	sub	@0x1234,@0x1345		; 0x2fff 0x0bfe 0x0d0d

	and	r1,r2			; 0x3042
	and	@r2,@r3			; 0x328b
	and	(r3)+,(r4)+		; 0x34d4
	and	@(r4)+,@(r5)+		; 0x371d
	and	-(r5),-(r6)		; 0x3966
	and	@-(r6),@-(r1)		; 0x3bA9
	and	0x1234(r1),0x1345(r2)	; 0x3c72 0x1234 0x1345
	and	@0x1234(r2),@0x1345(r3)	; 0x3ebb 0x1234 0x1345
	expect	1350
	and	#0x1234,#0x1345
	endexpect
	and	#0x1234,r4		; 0x35c4 0x1234
	and	@#0x1234,@#0x1345	; 0x37df 0x1234 0x1345
	and	0x1234,0x1345		; 0x3df7 0x0bd6 0x0ce5
	and	@0x1234,@0x1345		; 0x3fff 0x0bd0 0x0cdf

	bic	r1,r2			; 0x4042
	bic	@r2,@r3			; 0x428b
	bic	(r3)+,(r4)+		; 0x44d4
	bic	@(r4)+,@(r5)+		; 0x471d
	bic	-(r5),-(r6)		; 0x4966
	bic	@-(r6),@-(r1)		; 0x4bA9
	bic	0x1234(r1),0x1345(r2)	; 0x4c72 1234 1345
	bic	@0x1234(r2),@0x1345(r3)	; 0x4ebb 1234 1345
	expect	1350
	bic	#0x1234,#0x1345
	endexpect
	bic	#0x1234,r4		; 0x45c4 0x1234
	bic	@#0x1234,@#0x1345	; 0x47df 0x1234 0x1345
	bic	0x1234,0x1345		; 0x4df7 0x0ba8 0x0cb7
	bic	@0x1234,@0x1345		; 0x4fff 0x0ba2 0x0cb1

	bis	r1,r2			; 0x5042
	bis	@r2,@r3			; 0x528b
	bis	(r3)+,(r4)+		; 0x54d4
	bis	@(r4)+,@(r5)+		; 0x571d
	bis	-(r5),-(r6)		; 0x5966
	bis	@-(r6),@-(r1)		; 0x5bA9
	bis	0x1234(r1),0x1345(r2)	; 0x5c72 0x1234 0x1345
	bis	@0x1234(r2),@0x1345(r3)	; 0x5ebb 0x1234 0x1345
	expect	1350
	bis	#0x1234,#0x1345
	endexpect
	bis	#0x1234,r4		; 0x55c4 0x1234
	bis	@#0x1234,@#0x1345	; 0x57df 0x1234 0x1345
	bis	0x1234,0x1345		; 0x5df7 0x0b7a 0x0c89
	bis	@0x1234,@0x1345		; 0x5fff 0x0b74 0x0c83

	xor	r1,r2			; 0x6042
	xor	@r2,@r3			; 0x628b
	xor	(r3)+,(r4)+		; 0x64d4
	xor	@(r4)+,@(r5)+		; 0x671d
	xor	-(r5),-(r6)		; 0x6966
	xor	@-(r6),@-(r1)		; 0x6bA9
	xor	0x1234(r1),0x1345(r2)	; 0x6c72 0x1234 0x1345
	xor	@0x1234(r2),@0x1345(r3)	; 0x6ebb 0x1234 0x1345
	expect	1350
	xor	#0x1234,#0x1345
	endexpect
	xor	#0x1234,r4		; 0x65c4 0x1234
	xor	@#0x1234,@#0x1345	; 0x67df 0x1234 0x1345
	xor	0x1234,0x1345		; 0x6df7 0x0b4c 0x0c5b
	xor	@0x1234,@0x1345		; 0x6fff 0x0b46 0x0c55

	cmp	r1,r2			; 0x9042
	cmp	@r2,@r3			; 0x928b
	cmp	(r3)+,(r4)+		; 0x94d4
	cmp	@(r4)+,@(r5)+		; 0x971d
	cmp	-(r5),-(r6)		; 0x9966
	cmp	@-(r6),@-(r1)		; 0x9bA9
	cmp	0x1234(r1),0x1345(r2)	; 0x9c72 0x1234 0x1345
	cmp	@0x1234(r2),@0x1345(r3)	; 0x9ebb 0x1234 0x1345
	cmp	#0x1234,#0x1345		; 0x95d7 0x1234 0x1345
	cmp	#0x1234,r4		; 0x95c4 0x1234
	cmp	@#0x1234,@#0x1345	; 0x97df 0x1234 0x1345
	cmp	0x1234,0x1345		; 0x9df7 0x0b18 0x0c27
	cmp	@0x1234,@0x1345		; 0x9fff 0x0b12 0x0c21

	bit	r1,r2			; 0xa042
	bit	@r2,@r3			; 0xa28b
	bit	(r3)+,(r4)+		; 0xa4d4
	bit	@(r4)+,@(r5)+		; 0xa71d
	bit	-(r5),-(r6)		; 0xa966
	bit	@-(r6),@-(r1)		; 0xabA9
	bit	0x1234(r1),0x1345(r2)	; 0xac72 0x1234 0x1345
	bit	@0x1234(r2),@0x1345(r3)	; 0xaebb 0x1234 0x1345
	bit	#0x1234,#0x1345		; 0xa5d7 0x1234 0x1345
	bit	#0x1234,r4		; 0xa5c4 0x1234
	bit	@#0x1234,@#0x1345	; 0xa7df 0x1234 0x1345
	bit	0x1234,0x1345		; 0xadf7 0x0ae4 0x0bf3
	bit	@0x1234,@0x1345		; 0xafff 0x0ade 0x0bed

	mov	r1,r2			; 0xb042
	mov	@r2,@r3			; 0xb28b
	mov	(r3)+,(r4)+		; 0xb4d4
	mov	@(r4)+,@(r5)+		; 0xb71d
	mov	-(r5),-(r6)		; 0xb966
	mov	@-(r6),@-(r1)		; 0xbbA9
	mov	0x1234(r1),0x1345(r2)	; 0xbc72 0x1234 0x1345
	mov	@0x1234(r2),@0x1345(r3)	; 0xbebb 0x1234 0x1345
	expect	1350
	mov	#0x1234,#0x1345
	endexpect
	mov	#0x1234,r4		; 0xb5c4 0x1234
	mov	@#0x1234,@#0x1345	; 0xb7df 0x1234 0x1345
	mov	0x1234,0x1345		; 0xbdf7 0x0ab6 0x0bc5
	mov	@0x1234,@0x1345		; 0xbfff 0x0ab0 0x0bbf

	cmpb	r1,r2			; 0xc042
	cmpb	@r2,@r3			; 0xc28b
	cmpb	(r3)+,(r4)+		; 0xc4d4
	cmpb	@(r4)+,@(r5)+		; 0xc71d
	cmpb	-(r5),-(r6)		; 0xc966
	cmpb	@-(r6),@-(r1)		; 0xcbA9
	cmpb	0x1234(r1),0x1345(r2)	; 0xcc72 0x1234 0x1345
	cmpb	@0x1234(r2),@0x1345(r3)	; 0xcebb 0x1234 0x1345
	cmpb	#0x12,#0x13		; 0xc5d7 0x0012 0x0013
	cmpb	#0x12,r4		; 0xc5c4 0x0012
	cmpb	@#0x1234,@#0x1345	; 0xc7df 0x1234 0x1345
	cmpb	0x1234,0x1345		; 0xcdf7 0x0a82 0x0b91
	cmpb	@0x1234,@0x1345		; 0xcfff 0x0a7c 0x0b8b

	movb	r1,r2			; 0xd042
	movb	@r2,@r3			; 0xd28b
	movb	(r3)+,(r4)+		; 0xd4d4
	movb	@(r4)+,@(r5)+		; 0xd71d
	movb	-(r5),-(r6)		; 0xd966
	movb	@-(r6),@-(r1)		; 0xdbA9
	movb	0x1234(r1),0x1345(r2)	; 0xdc72 0x1234 0x1345
	movb	@0x1234(r2),@0x1345(r3)	; 0xdebb 0x1234 0x1345
	expect	1350
	movb	#0x12,#0x13
	endexpect
	movb	#0x12,r4		; 0xd5c4 0x0012
	movb	@#0x1234,@#0x1345	; 0xd7df 0x1234 0x1345
	movb	0x1234,0x1345		; 0xddf7 0x0a54 0x0b63
	movb	@0x1234,@0x1345		; 0xdfff 0x0a4e 0x0b5d

	bisb	r1,r2			; 0xe042
	bisb	@r2,@r3			; 0xe28b
	bisb	(r3)+,(r4)+		; 0xe4d4
	bisb	@(r4)+,@(r5)+		; 0xe71d
	bisb	-(r5),-(r6)		; 0xe966
	bisb	@-(r6),@-(r1)		; 0xebA9
	bisb	0x1234(r1),0x1345(r2)	; 0xec72 0x1234 0x1345
	bisb	@0x1234(r2),@0x1345(r3)	; 0xeebb 0x1234 0x1345
	expect	1350
	bisb	#0x12,#0x13
	endexpect
	bisb	#0x12,r4		; 0xe5c4 0x1234
	bisb	@#0x1234,@#0x1345	; 0xe7df 0x1234 0x1345
	bisb	0x1234,0x1345		; 0xedf7 0x0a26 0x0b35
	bisb	@0x1234,@0x1345		; 0xefff 0x0a20 0x0b2f

	; note that (Rn) without displacement is treated
	; like @Rn, so either is allowed here for mode FP0:

	fadd	@r2,(r3)		; 0xf023
	fadd	(r2),@(r3)		; 0xf02b
	fadd	@(r2),@r3		; 0xf0a3
	fadd	@(r2),@(r3)		; 0xf0ab
	expect	1350
	fadd	@r2,(r4)+
	endexpect

	fsub	@r2,(r3)		; 0xf123
	fsub	(r2),@(r3)		; 0xf12b
	fsub	@(r2),@r3		; 0xf1a3
	fsub	@(r2),@(r3)		; 0xf1ab
	expect	1350
	fsub	@r2,(r4)+
	endexpect

	fmul	@r2,(r3)		; 0xf223
	fmul	(r2),@(r3)		; 0xf22b
	fmul	@(r2),@r3		; 0xf2a3
	fmul	@(r2),@(r3)		; 0xf2ab
	expect	1350
	fmul	@r2,(r4)+
	endexpect

	fdiv	@r2,(r3)		; 0xf323
	fdiv	(r2),@(r3)		; 0xf32b
	fdiv	@(r2),@r3		; 0xf3a3
	fdiv	@(r2),@(r3)		; 0xf3ab
	expect	1350
	fdiv	@r2,(r4)+
	endexpect

	fcmp	@r2,(r3)		; 0xf423
	fcmp	(r2),@(r3)		; 0xf42b
	fcmp	@(r2),@r3		; 0xf4a3
	fcmp	@(r2),@(r3)		; 0xf4ab
	expect	1350
	fcmp	@r2,(r4)+
	endexpect

	; The assumption is that the WD-16's
	; floating point format is 'halfway'
	; between the DEC/PDP-11 F and D format,
	; i.e. the F format is only extended
	; by 16 mantissa bits:

	flt3	1.0

	; Since the WD16 is effectively an LSI-11 with different
        ; microcode, it also implements multi character constants
        ; in little endian mode.  So this  results in text in
        ; memory that is not byte swapped:

	word	'Th','e ','qu','ic','k ','br','ow','n ','fo'
	word	'x ','ju','mp','s ','ov','er',' t','he',' l'
	word	'az','y ','do','g.'
