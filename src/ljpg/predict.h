/*
 * predict.h --
 *
 * Code for predictor calculation. Its function version, predictor.c,
 * is used in debugging compilation.
 */
/*
 * $Id: predict.h,v 1.1 2005/07/28 19:25:50 oph Exp $
 */
#ifndef _PREDICTOR
#define _PREDICTOR

#ifndef DEBUG

/*
 *--------------------------------------------------------------
 *
 * Predict --
 *
 *      Calculate the predictor for pixel[row][col][curComp],
 *      i.e. curRowBuf[col][curComp]. It handles the all special 
 *      cases at image edges, such as first row and first column
 *      of a scan.
 *
 * Results:
 *      predictor is passed out.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */
#define Predict(row,col,curComp,curRowBuf,prevRowBuf,Pr,Pt,psv,predictor) \
{    register int left,upper,diag,leftcol;                                \
                                                                          \
    leftcol=col-1;                                                        \
    if (row==0) {                                                         \
                                                                          \
       /*                                                                 \
        * The predictor of first pixel is (1<<(Pr-Pt-1), and the          \
        * predictors for rest of first row are left neighbors.            \
        */                                                                \
       if (col==0) {                                                      \
          *predictor = (1<<(Pr-Pt-1));                                    \
       }                                                                  \
       else {                                                             \
          *predictor = curRowBuf[leftcol][curComp];                       \
       }                                                                  \
    }                                                                     \
    else {                                                                \
                                                                          \
       /*                                                                 \
        * The predictors of first column are upper neighbors.             \
        * All other preditors are calculated according to psv.            \
        */                                                                \
       upper=prevRowBuf[col][curComp];                                    \
       if (col==0)                                                        \
          *predictor = upper;                                             \
       else {                                                             \
          left=curRowBuf[leftcol][curComp];                               \
          diag=prevRowBuf[leftcol][curComp];                              \
          switch (psv) {                                                  \
             case 0:                                                      \
                     *predictor = 0;                                      \
                     break;                                               \
             case 1:                                                      \
                     *predictor = left;                                   \
                     break;                                               \
             case 2:                                                      \
                     *predictor = upper;                                  \
                     break;                                               \
             case 3:                                                      \
                     *predictor = diag;                                   \
                     break;                                               \
             case 4:                                                      \
                     *predictor = left+upper-diag;                        \
                     break;                                               \
             case 5:                                                      \
                     *predictor = left+((upper-diag)>>1);                 \
                     break;                                               \
             case 6:                                                      \
                     *predictor = upper+((left-diag)>>1);                 \
                     break;                                               \
             case 7:                                                      \
                     *predictor = (left+upper)>>1;                        \
                     break;                                               \
             default:                                                     \
                     fprintf(stderr,"Warning: Undefined PSV\n");          \
                     *predictor = 0;                                      \
           }                                                              \
        }                                                                 \
      }                                                                   \
}

/*
 *--------------------------------------------------------------
 *
 * QuickPredict --
 *
 *      Calculate the predictor for sample curRowBuf[col][curComp].
 *        It does not handle the special cases at image edges, such 
 *      as first row and first column of a scan. We put the special 
 *        case checkings outside so that the computations in main
 *        loop can be simpler. This has enhenced the performance
 *        significantly.
 *
 * Results:
 *      predictor is passed out.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */
#define QuickPredict(col,curComp,curRowBuf,prevRowBuf,psv,predictor){     \
    register int left,upper,diag,leftcol;                                 \
                                                                          \
    /*                                                                    \
     * All predictor are calculated according to psv.                     \
     */                                                                   \
    switch (psv) {                                                        \
      case 0:                                                             \
              *predictor = 0;                                             \
              break;                                                      \
      case 1:                                                             \
              *predictor = curRowBuf [col-1] [curComp];                   \
              break;                                                      \
      case 2:                                                             \
              *predictor = prevRowBuf[col][curComp];                      \
              break;                                                      \
      case 3:                                                             \
              *predictor = prevRowBuf [col-1] [curComp];                  \
              break;                                                      \
      case 4:                                                             \
                  leftcol = col-1;                                        \
                  upper   = prevRowBuf[col][curComp];                     \
                  left    = curRowBuf[leftcol][curComp];                  \
                  diag    = prevRowBuf[leftcol][curComp];                 \
              *predictor = left + upper - diag;                           \
              break;                                                      \
      case 5:                                                             \
                  leftcol = col-1;                                        \
                  upper   = prevRowBuf[col][curComp];                     \
                  left    = curRowBuf[leftcol][curComp];                  \
                  diag    = prevRowBuf[leftcol][curComp];                 \
              *predictor = left+((upper-diag)>>1);                        \
              break;                                                      \
      case 6:                                                             \
                  leftcol = col-1;                                        \
                  upper   = prevRowBuf[col][curComp];                     \
                  left    = curRowBuf[leftcol][curComp];                  \
                  diag    = prevRowBuf[leftcol][curComp];                 \
              *predictor = upper+((left-diag)>>1);                        \
              break;                                                      \
      case 7:                                                             \
                  leftcol = col-1;                                        \
                  upper   = prevRowBuf[col][curComp];                     \
                  left    = curRowBuf[leftcol][curComp];                  \
              *predictor = (left+upper)>>1;                               \
              break;                                                      \
      default:                                                            \
              fprintf(stderr,"Warning: Undefined PSV\n");                 \
              *predictor = 0;                                             \
     }                                                                    \
}

#endif /* DEBUG */
#endif /* _PREDICTOR */
