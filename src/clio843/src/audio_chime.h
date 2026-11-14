/* Bounded integer PCM cue, shared by the real receiver and native verification.
 * Deep two-tone camera cue: 520 Hz then 390 Hz with a soft second harmonic.
 * Short attack, gentle release, and a gradual music duck/release. The caller
 * passes a 0..256 volume so the same cue follows the user's setting. */
struct AudioChime {unsigned rate,pos,total,phase,inc1,inc2,ramp,rel,vol;};
static void chime_start(struct AudioChime*c,unsigned rate,unsigned vol){c->rate=rate;c->pos=0;c->total=rate*45/100;c->phase=0;c->inc1=udiv32(520u*65536u,rate);c->inc2=udiv32(390u*65536u,rate);c->ramp=rate/100;if(!c->ramp)c->ramp=1;c->rel=rate*7/100;if(!c->rel)c->rel=1;c->vol=vol>256u?256u:vol;}
static int chime_active(const struct AudioChime*c){return c->pos<c->total;}
static int chime_sample(struct AudioChime*c,int*gain){
 static const signed short sine[32]={0,6393,12539,18204,23170,27245,30273,32137,32767,32137,30273,27245,23170,18204,12539,6393,0,-6393,-12539,-18204,-23170,-27245,-30273,-32137,-32767,-32137,-30273,-27245,-23170,-18204,-12539,-6393};
 if(!chime_active(c)){*gain=256;return 0;}unsigned p=c->pos++,r=c->rate,env=256;
 if(p<c->ramp)env=udiv32(p*256,c->ramp);
 if(c->total-p<c->rel)env=udiv32((c->total-p)*256,c->rel);
 *gain=256-(int)((env*176)>>8);unsigned begin=0,end=r*15/100;int tone=p<end;
 if(p>=r*21/100&&p<r*43/100){begin=r*21/100;end=r*43/100;tone=1;}
 if(!tone)return 0;unsigned a=p-begin,b=end-p;env=256;
 if(a<c->ramp)env=udiv32(a*256,c->ramp);if(b<c->rel)env=udiv32(b*256,c->rel);
 c->phase+=begin?c->inc2:c->inc1;
 unsigned i=(c->phase>>11)&31,f=c->phase&2047;int wave=sine[i]+((sine[(i+1)&31]-sine[i])*(int)f)/2048;
 unsigned h=c->phase*2u;unsigned j=(h>>11)&31,g=h&2047;int hi=sine[j]+((sine[(j+1)&31]-sine[j])*(int)g)/2048;
 int v=((wave+(hi>>2))*(int)env)>>10;
 return (v*(int)c->vol)>>8;
}
static int chime_mix(int sample,int gain,int tone){int v=(sample*gain)/256+tone;return v>32767?32767:v< -32768?-32768:v;}
