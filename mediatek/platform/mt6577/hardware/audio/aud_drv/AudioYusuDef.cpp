



#include "AudioYusuDef.h"

#if defined(PC_EMULATION)
#include "windows.h"
#else
#include "unistd.h"
#include "pthread.h"
#endif
extern "C" {
#include "bli_exp.h"
}





namespace android {

void AAD_Sleep_1ms(int ms)
{
#if defined(PC_EMULATION)
   Sleep(1 * ms);
#else
   usleep(1000 * ms);
#endif
}

int rb_getDataCount(rb *rb1)
{
   int count = rb1->pWrite - rb1->pRead;
   if( count < 0 ) count += rb1->bufLen;
   return count;
}

int rb_getFreeSpace(rb *rb1)
{
   int count = rb1->pRead - rb1->pWrite - 1;
   if( count < 0 ) count += rb1->bufLen;
   return count;
}


void rb_copyToLinear(char *buf, rb *rb1, int count)
{
	
   if ( rb1->pRead <= rb1->pWrite ) {
      memcpy(buf, rb1->pRead, count);
      rb1->pRead += count;
   } 
   else {
      char *end = rb1->pBufBase + rb1->bufLen;
      int r2e = (int)end - (int)rb1->pRead;
      if ( count <= r2e ) {
         memcpy(buf, rb1->pRead, count);
         rb1->pRead += count;
         if ( rb1->pRead == end )
            rb1->pRead = rb1->pBufBase;
      }
      else {
         memcpy(buf, rb1->pRead, r2e);
         memcpy(buf + r2e, rb1->pBufBase, count - r2e);
         rb1->pRead = rb1->pBufBase + count - r2e;
      }
   }

}

void rb_copyFromLinear(rb *rb1, char *buf, int count)
{
   int spaceIHave;
   char *end = rb1->pBufBase + rb1->bufLen;

   // count buffer data I have
   spaceIHave = rb1->bufLen - rb_getDataCount(rb1) - 1;
   //spaceIHave = rb_getDataCount(rb1);

   // if not enough, assert
   ASSERT(spaceIHave >= count);

   if ( rb1->pRead <= rb1->pWrite ) {
      int w2e = (int)end - (int)rb1->pWrite;
      if ( count <= w2e ) {
         memcpy(rb1->pWrite, buf, count);
         rb1->pWrite += count;
         if ( rb1->pWrite == end ) 
            rb1->pWrite = rb1->pBufBase;
      }
      else {
         memcpy(rb1->pWrite, buf, w2e);
         memcpy(rb1->pBufBase, buf + w2e, count - w2e);
         rb1->pWrite = rb1->pBufBase + count - w2e;         
      } 
   }
   else {
      memcpy(rb1->pWrite, buf, count);
      rb1->pWrite += count;
   }

}

void rb_copyEmpty(rb *rbt, rb *rbs)
{
   if ( rbs->pRead <= rbs->pWrite ) {
      rb_copyFromLinear(rbt, rbs->pRead, rbs->pWrite - rbs->pRead);
      //rbs->pRead = rbs->pWrite;
      // no need to update source read pointer, because it is read to empty
   }
   else {
      char *end = rbs->pBufBase + rbs->bufLen;
      rb_copyFromLinear(rbt, rbs->pRead, end - rbs->pRead);
      rb_copyFromLinear(rbt, rbs->pBufBase, rbs->pWrite - rbs->pBufBase);
   }   
}

int32 rb_copyFromRb(rb *rbt, rb *rbs, int32 count)
{
   int cntInRbs = rb_getDataCount(rbs);
   int cntInRbt = rb_getDataCount(rbt);
   ASSERT(count <= cntInRbs && count <= cntInRbt );


   if ( rbs->pRead <= rbs->pWrite ) {
      rb_copyFromLinear(rbt, rbs->pRead, count);
      rbs->pRead += count;
      // no need to update source read pointer, because it is read to empty
   }
   else {
      char *end = rbs->pBufBase + rbs->bufLen;
      int cnt2e = end - rbs->pRead;
      if ( cnt2e >= count ) {
         rb_copyFromLinear(rbt, rbs->pRead, count);
         rbs->pRead += count;
         if (rbs->pRead == end) 
            rbs->pRead = rbs->pBufBase;
      }
      else {
         rb_copyFromLinear(rbt, rbs->pRead, cnt2e);
         rb_copyFromLinear(rbt, rbs->pBufBase, count - cnt2e);
         rbs->pRead = rbs->pBufBase + count - cnt2e;
      }
   }
   return count;
}

// assuming ring buffer has enough space
void rb_writeDataValue(rb *rb1, int32 value, int32 count)
{
   int spaceIHave;

   // count buffer data I have
   spaceIHave = rb1->bufLen - rb_getDataCount(rb1) - 1;

   // if not enough, assert
   ASSERT(spaceIHave >= count);

   if ( rb1->pRead <= rb1->pWrite ) {
      char *end = rb1->pBufBase + rb1->bufLen;
      int w2e = (int)end - (int)rb1->pWrite;
      if ( count <= w2e ) {
         memset(rb1->pWrite, value, count);
         rb1->pWrite += count;
      }
      else {
         memset(rb1->pWrite, value, w2e);
         memset(rb1->pBufBase, value, count - w2e);
         rb1->pWrite = rb1->pBufBase + count - w2e;         
      } 
   }
   else {
      memset(rb1->pWrite, value, count);
      rb1->pWrite += count;
   }

}

//----------following ring buffer operation with bli convert---------


// keep on trying until taget source ring buffer is empty (with SRC)
void rb_copyFromLinearSRC(void *pSrcHdl, rb *rbt, int8 *buf, int32 num, int32 srt, int32 srs)
{
   uint32 inCount, outCount, consumed;
   uint32 tryCount = 0;

   //num not consumed
   int32 leftCount = num;

   //keep on trying until leftCount = 0
   do {
      
      if( rbt->pRead <= rbt->pWrite ) {  
         //printf("glbcnt: %d\n", glbcnt);
         //if ( glbcnt == 3 ) 
         //   glbcnt = glbcnt;
         //glbcnt++;
         
         //If write pointer is larger than read, we output from write til end
         inCount = leftCount;
         if ( rbt->pRead == rbt->pBufBase )
            outCount = rbt->pBufBase + rbt->bufLen - rbt->pWrite - 2;
         else
            outCount = rbt->pBufBase + rbt->bufLen - rbt->pWrite;
         
         //Do Conversion
         consumed = BLI_Convert(pSrcHdl, (int16*)buf, &inCount, (int16*)rbt->pWrite, &outCount);
         buf += consumed;
         
         //Judge if we need to try consume again
         if ( inCount == 0 ) {         // all input buffer is consumed, means no data to copy to rb
            rbt->pWrite += outCount;   // update write pointer with outcount
            break;                     
         }
         else {                           // input buffer is not completely consumed, but ring buffer hits its end
            rbt->pWrite += outCount;
            if ( rbt->pWrite == rbt->pBufBase + rbt->bufLen )
               rbt->pWrite = rbt->pBufBase;  // ring buffer write pointer start from base
            leftCount -= consumed;        // update leftCount try to consume again
         }

      
      //   printf(" pRead:%d, pWrite:%d, pBase:0x%x, pEnd:0x%x\n", 
      //      rbt->pRead - rbt->pBufBase, rbt->pWrite - rbt->pBufBase, rbt->pBufBase, rbt->pBufBase+ rbt->bufLen);
         
      }
      else {         
         //If read pointer is larger than write, we output from write til read pointer
         inCount = leftCount;
         outCount = rbt->pRead - rbt->pWrite - 2;
         
         //Do Conversion
         consumed = BLI_Convert(pSrcHdl, (int16*)buf, &inCount, (int16*)rbt->pWrite, &outCount);
         buf += consumed;
         
         //Judge if we need to try consume again
         if ( inCount == 0 ) {         // all input buffer is consumed, means no data to copy to rb
            rbt->pWrite += outCount;   // update write pointer with outcount
            break;                     
         }
         else {                           // input buffer is not completely consumed, but ring buffer hits read pointer
            rbt->pWrite += outCount;      // update write pointer
            leftCount -= consumed;        // update leftCount try to consume again
         }
      //   printf(" pRead:%d, pWrite:%d, pBase:0x%x, pEnd:0x%x\n", 
      //      rbt->pRead - rbt->pBufBase, rbt->pWrite - rbt->pBufBase, rbt->pBufBase, rbt->pBufBase+ rbt->bufLen);
      }
      
   } while ( tryCount++ < 200 );
   
      //   printf("exit:-- pRead:%d, pWrite:%d, pBase:0x%x, pEnd:0x%x\n", 
      //      rbt->pRead - rbt->pBufBase, rbt->pWrite - rbt->pBufBase, rbt->pBufBase, rbt->pBufBase+ rbt->bufLen);
   
   if(tryCount >= 20) ASSERT(0);
   
}

void rb_copyEmptySRC(void *pSrcHdl, rb *rbt, rb *rbs, int32 srt, int32 srs)
{
   if ( rbs->pRead <= rbs->pWrite ) {
      rb_copyFromLinearSRC(pSrcHdl, rbt, rbs->pRead, rbs->pWrite - rbs->pRead, srt, srs);
      //rbs->pRead = rbs->pWrite;
      // no need to update source read pointer, because it is read to empty
   }
   else {
      char *end = rbs->pBufBase + rbs->bufLen;
      rb_copyFromLinearSRC(pSrcHdl, rbt, rbs->pRead, end - rbs->pRead, srt, srs);
      rb_copyFromLinearSRC(pSrcHdl, rbt, rbs->pBufBase, rbs->pWrite - rbs->pBufBase, srt, srs);
   }   
}

}   //android

