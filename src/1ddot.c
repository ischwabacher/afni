#include "mrilib.h"

int main( int argc , char *argv[] )
{
   int iarg , ii,jj,kk,mm , nvec , do_one=0 , nx=0,ny ;
   MRI_IMAGE *tim ;
   MRI_IMARR *tar ;
   double sum , *eval , *amat , **tvec , *bmat ;
   float *far ;

   /* help? */

   if( argc < 2 || strcmp(argv[1],"-help") == 0 ){
     printf("Usage: 1ddot [options] 1Dfile 1Dfile ...\n"
            "- Prints out correlation matrix of the 1D files and\n"
            "  their inverse correlation matrix.\n"
            "- Output appears on stdout.\n"
            "\n"
            "Options:\n"
            " -one  =  Make 1st vector be all 1's.\n"
           ) ;
     exit(0) ;
   }

   /* options */

   iarg = 1 ; nvec = 0 ;
   while( iarg < argc && argv[iarg][0] == '-' ){

     if( strcmp(argv[iarg],"-one") == 0 ){
       do_one = 1 ; iarg++ ; continue ;
     }

     fprintf(stderr,"** Unknown option: %s\n",argv[iarg]); exit(1);
   }

   if( iarg == argc ){
     fprintf(stderr,"** No 1D files on command line!?\n"); exit(1);
   }

   /* input 1D files */

   INIT_IMARR(tar) ; if( do_one ) nvec = 1 ;
   for( ; iarg < argc ; iarg++ ){
     tim = mri_read_1D( argv[iarg] ) ;
     if( tim == NULL ){
       fprintf(stderr,"** Can't read 1D file %s\n",argv[iarg]); exit(1);
     }
     if( nx == 0 ){
       nx = tim->nx ;
     } else if( tim->nx != nx ){
       fprintf(stderr,"** 1D file %s doesn't match first file in length!\n",
               argv[iarg]); exit(1);
     }
     nvec += tim->ny ;
     ADDTO_IMARR(tar,tim) ;
   }

   /* create normalized vectors from 1D files */

   tvec = (double **) malloc( sizeof(double *)*nvec ) ;
   for( jj=0 ; jj < nvec ; jj++ )
     tvec[jj] = (double *) malloc( sizeof(double)*nx ) ;

   kk = 0 ;
   if( do_one ){
     sum = 1.0 / sqrt((double)nx) ;
     for( ii=0 ; ii < nx ; ii++ ) tvec[0][ii] = sum ;
     kk = 1 ;
   }

   for( mm=0 ; mm < IMARR_COUNT(tar) ; mm++ ){
     tim = IMARR_SUBIM(tar,mm) ;
     far = MRI_FLOAT_PTR(tim) ;
     for( jj=0 ; jj < tim->ny ; jj++,kk++ ){
       sum = 0.0 ;
       for( ii=0 ; ii < nx ; ii++ ){
         tvec[kk][ii] = far[ii+jj*nx] ;
         sum += tvec[kk][ii] * tvec[kk][ii] ;
       }
       if( sum == 0.0 ){
         fprintf(stderr,"** Input column is all zero!\n"); exit(1);
       }
       sum = 1.0 / sqrt(sum) ;
       for( ii=0 ; ii < nx ; ii++ ) tvec[kk][ii] *= sum ;
     }
   }
   DESTROY_IMARR(tar) ;

   /* create matrix from dot product of vectors */

   amat = (double *) calloc( sizeof(double) , nvec*nvec ) ;

   for( kk=0 ; kk < nvec ; kk++ ){
     for( jj=0 ; jj <= kk ; jj++ ){
       sum = 0.0 ;
       for( ii=0 ; ii < nx ; ii++ ) sum += tvec[jj][ii] * tvec[kk][ii] ;
       amat[jj+nvec*kk] = sum ;
       if( jj < kk ) amat[kk+nvec*jj] = sum ;
     }
   }

   for( jj=0 ; jj < nvec ; jj++ ) free(tvec[jj]) ;
   free(tvec) ;

   /* print matrix out */

   printf("\n") ;
   printf("++ Correlation Matrix:\n   ") ;
   for( jj=0 ; jj < nvec ; jj++ ) printf("    %02d    ",jj) ;
   printf("\n   ") ;
   for( jj=0 ; jj < nvec ; jj++ ) printf(" ---------") ;
   printf("\n") ;
   for( kk=0 ; kk < nvec ; kk++ ){
     printf("%02d:",kk) ;
     for( jj=0 ; jj < nvec ; jj++ ) printf(" %9.6f",amat[jj+kk*nvec]) ;
     printf("\n") ;
   }

   /* compute eigendecomposition */

   eval = (double *) malloc( sizeof(double)*nvec ) ;
   symeig_double( nvec , amat , eval ) ;

   printf("\n"
          "++ Eigensolution:\n   " ) ;
   for( jj=0 ; jj < nvec ; jj++ ) printf(" %9.6f",eval[jj]) ;
   printf("\n   ") ;
   for( jj=0 ; jj < nvec ; jj++ ) printf(" ---------") ;
   printf("\n") ;
   for( kk=0 ; kk < nvec ; kk++ ){
     printf("%02d:",kk) ;
     for( jj=0 ; jj < nvec ; jj++ ) printf(" %9.6f",amat[kk+jj*nvec]) ;
     printf("\n") ;
   }

   /* compute matrix inverse */

   for( jj=0 ; jj < nvec ; jj++ ) eval[jj] = 1.0 / eval[jj] ;

   bmat = (double *) calloc( sizeof(double) , nvec*nvec ) ;

   for( ii=0 ; ii < nvec ; ii++ ){
     for( jj=0 ; jj < nvec ; jj++ ){
       sum = 0.0 ;
       for( kk=0 ; kk < nvec ; kk++ )
         sum += amat[ii+kk*nvec] * amat[jj+kk*nvec] * eval[kk] ;
       bmat[ii+jj*nvec] = sum ;
     }
   }

   printf("\n") ;
   printf("++ Correlation Matrix Inverse:\n   ") ;
   for( jj=0 ; jj < nvec ; jj++ ) printf("    %02d    ",jj) ;
   printf("\n   ") ;
   for( jj=0 ; jj < nvec ; jj++ ) printf(" ---------") ;
   printf("\n") ;
   for( kk=0 ; kk < nvec ; kk++ ){
     printf("%02d:",kk) ;
     for( jj=0 ; jj < nvec ; jj++ ) printf(" %9.6f",bmat[jj+kk*nvec]) ;
     printf("\n") ;
   }

   /* normalize matrix inverse */

   for( ii=0 ; ii < nvec ; ii++ ){
     for( jj=0 ; jj < nvec ; jj++ ){
       sum = bmat[ii+ii*nvec] * bmat[jj+jj*nvec] ;
       if( sum > 0.0 )
         amat[ii+jj*nvec] = bmat[ii+jj*nvec] / sqrt(sum) ;
       else
         amat[ii+jj*nvec] = 0.0 ;
     }
   }

   printf("\n") ;
   printf("++ Correlation Matrix Inverse Normalized:\n   ") ;
   for( jj=0 ; jj < nvec ; jj++ ) printf("    %02d    ",jj) ;
   printf("\n   ") ;
   for( jj=0 ; jj < nvec ; jj++ ) printf(" ---------") ;
   printf("\n") ;
   for( kk=0 ; kk < nvec ; kk++ ){
     printf("%02d:",kk) ;
     for( jj=0 ; jj < nvec ; jj++ ) printf(" %9.6f",amat[jj+kk*nvec]) ;
     printf("\n") ;
   }

   /* done */

   exit(0) ;
}
