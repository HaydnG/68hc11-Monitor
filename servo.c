/* SG90 Servo driver cycle -90 degree to +90 degree */
void main(void)
{
unsigned int *tcnt,*toc2,delay1, delay2,dwait;
unsigned char *tflg1,*tctl1,*pgddr,*pgdr;
tctl1=(unsigned char*)0x20;
tflg1=(unsigned char*)0x23;
toc2=(unsigned int*)0x18;
tcnt=(unsigned int*)0x0e;
pgddr=(unsigned char*)0x3;
pgdr=(unsigned char*)0x02;
/* Define Port G as all outputs */
*pgddr =0xff;
/* Allow Port A not be used by timer */
*tctl1=0x00;
/* Repeat forever */
for(;;)
{
/* Need to physical send a PWM for a fixed time */
	for (dwait = 0;dwait <= 0xf;dwait++)
	{
	/* Move forward + 90 degrees*/
	delay1 = 0x8ac0;  /* off (Space) time */
	delay2 = 0x0fa0;  /* on (Mark) time */
	*pgdr = 0x00; /* Turn PWM signal off */
	*tflg1=0x40; /*Clear TOC2 Flag*/
	*toc2=*tcnt+delay1; /*Read timer and add offset period*/
	while(((*tflg1)&0x40)==0); /*Wait for TOC2 FLAG */
	*pgdr = 0x1; /* Turn PWM signal on */
	*tflg1=0x40; /*Clear TOC2 Flag*/
	*toc2=*tcnt+delay2; /*Read timer and add offset period */
	while(((*tflg1)&0x40)==0); /*Wait for TOC2 FLAG */
	}

/* Need to physical send a PWM for a fixed time */
	for (dwait = 0;dwait <= 0xf;dwait++)
	{
	/* Move back  -90 degrees*/
	delay1 = 0x9470;  /* off (Space) time */
	delay2 = 0x07d0;  /* on (Mark) time */
	*pgdr = 0x00; /* Turn PWM signal off */
	*tflg1=0x40; /*Clear TOC2 Flag*/
	*toc2=*tcnt+delay1; /*Read timer and add offset period*/
	while(((*tflg1)&0x40)==0); /*Wait for TOC2 FLAG */
	*pgdr = 0x1; /* Turn PWM signal on */
	*tflg1=0x40; /*Clear TOC2 Flag*/
	*toc2=*tcnt+delay2; /*Read timer and add offset period */
	while(((*tflg1)&0x40)==0); /*Wait for TOC2 FLAG */
	}
}
}
