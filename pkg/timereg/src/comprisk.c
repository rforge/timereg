//#include <stdio.h>
#include <math.h>
#include "matrix.h"
	 
void itfit(times,Ntimes,x,censcode,cause,KMc,z,n,px,Nit,betaS,
score,hess,est,var,sim,antsim,rani,test,testOBS,Ut,simUt,weighted,
gamma,vargamma,semi,zsem,pg,trans,gamma2,CA,line,detail,biid,gamiid,resample,
timepow,clusters,antclust,timepowtest,silent,convc,weights,entry,trunkp,estimator,fixgamma,stratum,ordertime,robust)
double *times,*betaS,*x,*KMc,*z,*score,*hess,*est,*var,*test,*testOBS,
*Ut,*simUt,*gamma,*zsem,*gamma2,*biid,*gamiid,*vargamma,*timepow,
	*timepowtest,*convc,*weights,*entry,*trunkp;
int *n,*px,*Ntimes,*Nit,*cause,*censcode,*sim,*antsim,*rani,*weighted,
*semi,*pg,*trans,*CA,*line,*detail,*resample,*clusters,*antclust,*silent,*estimator,*fixgamma,*stratum,*ordertime,*robust;
{ // {{{
  // {{{ allocation and reading of data from R
  matrix *X,*cX,*A,*AI,*cumAt[*antclust],*VAR,*Z,*censX;
  vector *VdB,*risk,*SCORE,*W,*Y,*Gc,*CAUSE,*bhat,*pbhat,*beta,*xi,*censXv,
    *rr,*rowX,*difbeta,*qs,*bhatub,*betaub,*dcovs,*pcovs,*zi,*rowZ,*zgam,*vcumentry; 
  vector *cumhatA[*antclust],*cumA[*antclust],*bet1,*gam,*dp,*dp1,*dp2; 
  int clusterj,osilent,convt,ps,sing,c,i,j,k,l,s,it,convproblems=0; 
  double nrisk,time,sumscore,totrisk, 
	 *vcudif=calloc((*Ntimes)*(*px+1),sizeof(double)),
	 *cifentry=calloc((*n),sizeof(double)),
	 *cumentry=calloc((*n)*(*px+1),sizeof(double));
  float gasdev(),expdev(),ran1();
  ps=(*px); 

  if (*semi==0) { 
    osilent=silent[0]; silent[0]=0; 
    malloc_mat(*n,*px,X); malloc_mats(*n,*px,&cX,&censX,NULL);
    if (*trans==2) {malloc_mat(*n,*pg,Z);malloc_vecs(*pg,&zgam,&gam,&zi,&rowZ,NULL);}
    malloc_mats(ps,ps,&A,&AI,&VAR,NULL); 

    malloc_vecs(*n,&rr,&bhatub,&risk,&W,&Y,&Gc,&CAUSE,&bhat,&pbhat,NULL); 
    malloc_vecs(*px,&vcumentry,&bet1,&xi,&rowX,&censXv,NULL); 
    malloc_vecs(ps,&dp,&dp1,&dp2,&dcovs,&pcovs,&betaub,&VdB,&qs,&SCORE,&beta,
	       &difbeta,NULL); 

    for (i=0;i<*antclust;i++) {
      malloc_vec(ps,cumhatA[i]); malloc_vec(ps,cumA[i]); 
      malloc_mat(*Ntimes,ps,cumAt[i]);}

    for (c=0;c<ps;c++) VE(beta,c)=betaS[c]; 
    for (c=0;c<*px;c++) VE(bet1,c)=betaS[c]; 
//    if (*trans==0) {for (c=0;c<*pg;c++) VE(gam,c)=betaS[*px+c];}

    for (c=0;c<*n;c++) {VE(Gc,c)=KMc[c]; 
//	 if (trunkp[c]<1) withtrunc=1; cifentry[c]=0; 
         VE(CAUSE,c)=cause[c]; 
         for(j=0;j<*px;j++)  ME(X,c,j)=z[j*(*n)+c]; 
    }

    // }}}
	 

for (s=0;s<*Ntimes;s++)
{
   time=times[s]; est[s]=time; score[s]=time; var[s]=time;
   convt=1; 

//    if (withtrunc==1) Cpred(est,Ntimes,px,entry,n,cumentry); 
	
 for (j=0;j<*antclust;j++) { 
     vec_zeros(cumA[j]); vec_zeros(cumhatA[j]); 
 }

  for (it=0;it<*Nit;it++)
  {
   R_CheckUserInterrupt();
    totrisk=0; 

    for (j=0;j<*n;j++) { // {{{ computation of P1 and DP1  and observed response 
      VE(risk,j)=(x[j]>=time); 
      totrisk=totrisk+VE(risk,j);
      extract_row(X,j,xi); 
      VE(bhat,j)=vec_prod(xi,bet1); 
//      for(i=0;i<*px;i++)  VE(vcumentry,j)=cumentry[(i+1)*(*n)+j]; 

      if (*trans==1) {
	VE(pbhat,j)=1-exp(-VE(bhat,j));
	scl_vec_mult(1-VE(pbhat,j),xi,dp);
//	if (trunkp[j]<1) { cifentry[j]=1-exp(-vec_prod(xi,vcumentry)); }; 
      }
      if (*trans==2) {
	VE(pbhat,j)=1-exp(-exp(VE(bhat,j))); 
	scl_vec_mult((1-VE(pbhat,j))*exp(VE(bhat,j)),xi,dp); 
      }
      if (*trans==6) {
	VE(pbhat,j)=1-exp(-VE(bhat,j)); 
	scl_vec_mult((1-VE(pbhat,j)),xi,dp); 
      }
      if (*trans==3) {
	    VE(pbhat,j)=exp(VE(bhat,j))/(1+exp(VE(bhat,j))); 
	scl_vec_mult(exp(VE(bhat,j))/pow((1+exp(VE(bhat,j))),2),xi,dp);
      // VE(pbhat,j)=exp(-VE(bhat,j)); scl_vec_mult(-exp(-VE(pbhat,j)),xi,dp);
      }
      if (*trans==4) {
         VE(pbhat,j)=exp(VE(bhat,j)); 
	 scl_vec_mult(exp(VE(bhat,j)),xi,dp);
      }
      if (*trans==5) {
         VE(pbhat,j)=VE(bhat,j); 
	 scl_vec_mult(1,xi,dp);
      }
      scl_vec_mult(1,dp,dp1); 

      if (*estimator<=2) scl_vec_mult(pow(weights[j],0.5)*(time>entry[j]),dp,dp); 
      else scl_vec_mult(pow(weights[j],0.5)*(time< KMc[j])*(time>entry[j]),dp,dp); 
      replace_row(cX,j,dp); 
//    replace_row(wcX,j,dp); 
      VE(Y,j)=((x[j]<=time) & (cause[j]==*CA))*1;

      if (it==(*Nit-1)) { // {{{ for censoring distrubution
	   if (KMc[j]>0.001) scl_vec_mult(weights[j]*VE(Y,j)*1/KMc[j],dp1,dp1); 
	   else scl_vec_mult(weights[j]*VE(Y,j)*1/0.001,dp1,dp1); 
           vec_add(censXv,dp1,censXv); 
           replace_row(censX,j,dp1);
      } // }}}

      if (*estimator==1) {
          if (KMc[j]<0.001) VE(Y,j)=((VE(Y,j)/0.001)-VE(pbhat,j)/trunkp[j])*(time>entry[j]); 
          else VE(Y,j)=( (VE(Y,j)/KMc[j])-VE(pbhat,j)/trunkp[j])*(time>entry[j]);
      } else if (*estimator==2) 
      {
          if (KMc[j]<0.001) VE(Y,j)=(1/0.001)*(VE(Y,j)-VE(pbhat,j)/trunkp[j])*(time>entry[j]); 
          else VE(Y,j)=(1/KMc[j])*(VE(Y,j)-VE(pbhat,j)/trunkp[j])*(time>entry[j]);
      } else VE(Y,j)=(VE(Y,j)-VE(pbhat,j)/trunkp[j])*(time<KMc[j])*(time>entry[j]);;
      VE(Y,j)=pow(weights[j],0.5)*VE(Y,j); 

    } // j=0;j<n*;j++ }}}

    totrisk=vec_sum(risk); 
    MtM(cX,A); 
    invertS(A,AI,osilent); sing=0; 
    // head_matrix(cX); print_mat(A); print_mat(AI); 
   if (ME(AI,0,0)==0 && *stratum==0 && (osilent==0)) {
	  Rprintf(" X'X not invertible at time %d %lf \n",s,time); 
	  print_mat(A); 
   }
   if (*stratum==1)  {
	for (k=0;k<*px;k++) if (fabs(ME(A,k,k))<0.000001)  ME(AI,k,k)=0; else ME(AI,k,k)=1/ME(A,k,k);
   }


    if (( fabs(ME(AI,0,0))<.0000001) && (*stratum==0)) {
      convproblems=1; convt=0; silent[s]=1;
      for (c=0;c<ps;c++) VE(beta,c)=0; 
      for (c=0;c<*px;c++) VE(bet1,c)=betaS[c]; 
      sing=1;
      if (osilent==0) Rprintf("Non-invertible design at time %lf \n",time); 
      it=*Nit-1;  
    }
    if (sing==0) {
      /* print_vec(Y); print_vec(SCORE); print_vec(difbeta); */ 
      vM(cX,Y,SCORE); 
      Mv(AI,SCORE,difbeta); vec_add(beta,difbeta,beta); 
      for (i=0;i<*px;i++) VE(bet1,i)=VE(beta,i); 

      sumscore=0; 
      for (k=0;k<*px;k++) sumscore=sumscore+fabs(VE(difbeta,k)); 
      if ((sumscore<*convc) & (it<*Nit-2)) it=*Nit-2;

      if (isnan(vec_sum(SCORE))) {
	Rprintf("missing values in SCORE %ld \n",(long int) s); 
	convproblems=1; convt=0; silent[s]=2;
	it=*Nit-1; 
	for (c=0;c<ps;c++) { VE(beta,c)=0; VE(SCORE,c)=99; }
	for (c=0;c<*px;c++) VE(bet1,c)=betaS[c]; 
	}
    }

    if (*detail==1) { 
      Rprintf("timepoint s %d, Estimate beta \n",s); print_vec(beta); 
      Rprintf("Score D l\n"); print_vec(difbeta); 
      Rprintf("Information -D^2 l\n"); print_mat(AI); 
    };
//
//    if ((it==(*Nit-1)) && (convt==1) && (*robust==0)) { // {{{ censoring terms for variance 
//	    for (j=0;j<*n;j++) { // sum from t->0 in cens times
//		k=ordertime[j];
//		nrisk=(*n)-j; 
//		clusterj=clusters[k]; 
//		if (cause[k]==(*censcode)) { 
//		   vec_add_mult(cumhatA[clusterj],censXv,1/nrisk,cumhatA[clusterj]);  
//	           for (i=j;i<*n;i++) {
//	                clusterj=clusters[ordertime[i]]; 	
//			vec_add_mult(cumhatA[clusterj],censXv,-1/pow(nrisk,2),cumhatA[clusterj]); 
//		   }
//		}
//                // fewer where I(s <= T_i) , because s is increasing
//                extract_row(censX,k,xi); 
//                vec_subtr(censXv,xi,censXv);  
//            }
//    } // }}} if (it==*Nit-1) if (convt==1 ) 
//

  } /* it */

   vec_zeros(VdB); mat_zeros(VAR); 

//    if (osilent<=1) for (i=0;i<*antclust;i++) vec_zeros(cumhatA[i]); 

if (convt==1 ) {
   for (i=0;i<*n;i++) { 
      R_CheckUserInterrupt();
      j=clusters[i]; 
      if (s<-1) Rprintf("%d  %d %d \n",s,i,j);
      for(k=0;k<ps;k++) VE(cumA[j],k)+= VE(Y,i)*ME(cX,i,k); 
      //  extract_row(cX,i,dp); 
      // scl_vec_mult(VE(Y,i),dp,dp); 
      //  vec_add(dp,cumA[j],cumA[j]); 

    if ((*robust==0)) { // {{{ censoring terms for variance 
		k=ordertime[i];
		nrisk=(*n)-i; 
		clusterj=clusters[k]; 
		if (cause[k]==(*censcode)) { 
                   for(k=0;k<ps;k++) VE(cumhatA[clusterj],k)+= VE(censXv,k)/nrisk; 
//		   vec_add_mult(cumhatA[clusterj],censXv,1/nrisk,cumhatA[clusterj]);  
		   scl_vec_mult(-1/pow(nrisk,2),censXv,rowX); 
	           for (j=i;j<*n;j++) {
	                clusterj=clusters[ordertime[j]]; 	
                        for(k=0;k<ps;k++) VE(cumhatA[clusterj],k)+= VE(rowX,k); 
//			vec_add(rowX,cumhatA[clusterj],cumhatA[clusterj]); 
		   }
		}
                // fewer where I(s <= T_i) , because s is increasing
                extract_row(censX,k,xi); 
                vec_subtr(censXv,xi,censXv);  
    } // }}}
   }

   for (j=0;j<*antclust;j++) { 
//    if (osilent<=2) else vec_subtr(cumhatA[j],cumA[j],dp1); 
      vec_add(cumhatA[j],cumA[j],dp1); 
      Mv(AI,dp1,dp2); replace_row(cumAt[j],s,dp2);  

      for(k=0;k<ps;k++) 
      for(c=0;c<ps;c++) ME(VAR,k,c)=ME(VAR,k,c)+VE(dp2,k)*VE(dp2,c); 

      if (*resample==1) {
      for (c=0;c<*px;c++) {l=j*(*px)+c; biid[l*(*Ntimes)+s]=VE(dp2,c);}
      }
   }
  }

   for (i=1;i<ps+1;i++) {
      var[i*(*Ntimes)+s]=ME(VAR,i-1,i-1); 
      est[i*(*Ntimes)+s]=VE(beta,i-1); score[i*(*Ntimes)+s]=VE(SCORE,i-1); }

} /* s=1 ... *Ntimes */ 

   R_CheckUserInterrupt();
    if (*sim==1)
      comptestfunc(times,Ntimes,px,est,var,vcudif,antsim,test,testOBS,Ut,
		   simUt,cumAt,weighted,antclust,gamma2,line,timepowtest); 
  } else {
    itfitsemi(times,Ntimes,x,censcode,cause,KMc,z,n,px,Nit,
	      score,hess,est,var,sim,antsim,rani,test,testOBS,Ut,simUt,weighted,
	      gamma,vargamma,semi,zsem,pg,trans,gamma2,CA,line,detail,biid,
	      gamiid,resample,timepow,clusters,antclust,timepowtest,silent,convc,weights,
	      entry,trunkp,estimator,fixgamma,stratum,ordertime,robust);
  }
 
  if (convproblems>0) convc[0]=1; 
  if (*semi==0) { 
    free_mats(&censX,&VAR,&X,&cX,&A,&AI,NULL); 
    if (*trans==2) {free_mats(&Z,NULL); free_vecs(&zgam,&gam,&zi,&rowZ,NULL);}

    free_vecs(&censXv,&rr,&bhatub,&risk,&W,&Y,&Gc,&CAUSE,&bhat,&pbhat,NULL); 
    free_vecs(&vcumentry,&bet1,&xi,&rowX,NULL); 
    free_vecs(&dp,&dp1,&dp2,&dcovs,&pcovs,&betaub,&VdB,&qs,&SCORE,&beta,&difbeta,NULL); 

    for (i=0;i<*antclust;i++) {free_vec(cumhatA[i]); free_vec(cumA[i]); 
    free_mat(cumAt[i]);}
  }
free(vcudif); free(cumentry); free(cifentry);  
} // }}}

void itfitsemi(times,Ntimes,x,censcode,cause,
	       KMc,z,antpers,px,Nit,
	       score,hess,est,var,sim,
	       antsim,rani,test,testOBS,Ut,
	       simUt,weighted,gamma,vargamma,semi,
	       zsem,pg,trans,gamma2,CA,
	       line,detail,biid,gamiid,resample,
	       timepow,clusters,antclust,timepowtest,silent,convc,weights,entry,trunkp,
	       estimator,fixgamma,stratum,ordertime,robust)
double *times,*x,*KMc,*z,*score,*hess,*est,*var,*test,*testOBS,*Ut,*simUt,*gamma,*zsem,
       *vargamma,*gamma2,*biid,*gamiid,*timepow,*timepowtest,*entry,*trunkp,*convc,*weights;
int *antpers,*px,*Ntimes,*Nit,*cause,*censcode,*sim,*antsim,*rani,*weighted,
*semi,*pg,*trans,*CA,*line,*detail,*resample,*clusters,*antclust,*silent,*estimator,*fixgamma,*stratum,*ordertime,*robust;
{ 
  // {{{ allocation and reading of data from R
  matrix *ldesignX,*A,*AI,*cdesignX,*ldesignG,*cdesignG,*censX,*censZ;
  matrix *S,*dCGam,*CGam,*ICGam,*VarKorG,*dC,*XZ,*ZZ,*ZZI,*XZAI; 
  matrix *Ct,*C[*Ntimes],*Acorb[*Ntimes],*tmpM1,*tmpM2,*tmpM3,*tmpM4; 
  matrix *Vargam,*dVargam,*M1M2[*Ntimes],*Delta,*dM1M2,*M1M2t,*RobVargam;
  matrix *W3t[*antclust],*W4t[*antclust];
//  matrix *W3tcens[*antclust],*W4tcens[*antclust];
  vector *W2[*antclust],*W3[*antclust];
//  vector *W2cens[*antclust],*W3cens[*antclust];
  vector *diag,*dB,*dN,*VdB,*AIXdN,*AIXlamt,*bhatt,*truncbhatt,*pbhat,*plamt,*ciftrunk;
  vector *korG,*pghat,*rowG,*gam,*dgam,*ZGdN,*IZGdN,*ZGlamt,*IZGlamt,*censZv,*censXv;
  vector *covsx,*covsz,*qs,*Y,*rr,*bhatub,*xi,*xit,*zit,*rowX,*rowZ,*difX,*zi,*z1,
    *tmpv1,*tmpv2,*lrisk;
  int sing,itt,i,j,k,l,s,c,pmax,totrisk,convproblems=0, 
      *n= calloc(1,sizeof(int)), *nx= calloc(1,sizeof(int)),
//      *robust= calloc(1,sizeof(int)),
      *px1= calloc(1,sizeof(int));
  int clusterj,fixedcov,osilent,withtrunc=0; 
  double nrisk,time,dummy,dtime,phattrunc,lrr,lrrt;
  double *vcudif=calloc((*Ntimes)*(*px+1),sizeof(double)),
	 *inc=calloc((*Ntimes)*(*px+1),sizeof(double)),
	 *cifentry=calloc((*antpers),sizeof(double)),
	 *cumentry=calloc((*antpers)*(*px+1),sizeof(double));
  osilent=silent[0]; silent[0]=0; 
  // float gasdev(),expdev(),ran1();
//  robust[0]=1; 
  fixedcov=1; n[0]=antpers[0]; nx[0]=antpers[0];

//if (*trans==1) for (j=0;j<*pg;j++) if (fabs(timepow[j]-1)>0.0001) {timem=1;break;}
//if (*trans==2) for (j=0;j<*pg;j++) if (fabs(timepow[j])>0.0001) {timem=1;break;}

  for (j=0;j<*antclust;j++) { 
    malloc_mat(*Ntimes,*px,W3t[j]);
    malloc_mat(*Ntimes,*px,W4t[j]); malloc_vec(*pg,W2[j]); 
    malloc_vec(*px,W3[j]);
  }

  malloc_mats(*antpers,*px,&censX,&ldesignX,&cdesignX,NULL);
  malloc_mats(*antpers,*pg,&censZ,&ldesignG,&cdesignG,NULL); 
  malloc_mats(*px,*px,&tmpM1,&A,&AI,NULL);
  malloc_mats(*pg,*pg,&dVargam,&Vargam,&RobVargam,&tmpM2,&ZZ,&VarKorG,&ICGam,&CGam,&dCGam,&S,&ZZI,NULL); 
  malloc_mats(*px,*pg,&XZAI,&tmpM3,&Ct,&dC,&XZ,&dM1M2,&M1M2t,NULL);
  malloc_mat(*px,*pg,tmpM4); 
  for (j=0;j<*Ntimes;j++) { malloc_mat(*pg,*px,Acorb[j]); 
    malloc_mat(*px,*pg,C[j]); malloc_mat(*px,*pg,M1M2[j]);}
  malloc_mat(*Ntimes,*px,Delta); malloc_mat(*Ntimes,*px,tmpM1);

  malloc_vecs(*px,&censXv,&covsx,&xit,&xi,&rowX,&difX,&tmpv1,&korG,&diag,&dB,&VdB,&AIXdN,&AIXlamt,&truncbhatt,&bhatt,NULL);
  malloc_vecs(*pg,&censZv,&covsz,&zit,&zi,&rowZ,&tmpv2,&zi,&z1,&rowG,&gam,&dgam,&ZGdN,&IZGdN,&ZGlamt,&IZGlamt,NULL);
  malloc_vecs(*antpers,&Y,&bhatub,&rr,&lrisk,&dN,&pbhat,&pghat,&plamt,&ciftrunk,NULL);
  malloc_vec((*px)+(*pg),qs); 

  if (*px>=*pg) pmax=*px; else pmax=*pg; 
  for (j=0;j<*pg;j++) VE(gam,j)=gamma[j]; 
  px1[0]=*px+1; 
  for (c=0;c<*antpers;c++) if (entry[c]>0) {withtrunc=1; break;}
  for(j=0;j<*antpers;j++)  for(i=0;i<=*px;i++) cumentry[i*(*antpers)+j]=0; 
  // }}}
  
  if (fixedcov==1) {
    for (c=0;c<*antpers;c++) {
      for(j=0;j<pmax;j++)  {
	if (j<*px) ME(ldesignX,c,j)= z[j*(*antpers)+c];
	if (j<*pg) ME(ldesignG,c,j)=zsem[j*(*antpers)+c]; } } }

  for (itt=0;itt<*Nit;itt++)
    {
   R_CheckUserInterrupt();
      mat_zeros(Ct); mat_zeros(CGam); vec_zeros(IZGdN); vec_zeros(IZGlamt); 

   if (withtrunc==1)  Cpred(est,Ntimes,px1,entry,antpers,cumentry); 

      Mv(ldesignG,gam,pghat);
      for (s=0;s<*Ntimes;s++)
      {
	  time=times[s]; if (s==0) dtime=0; else dtime=time-times[s-1]; 
//	  dtime=1; 

	  for(j=1;j<=*px;j++) VE(bhatt,j-1)=est[j*(*Ntimes)+s];
	  Mv(ldesignX,bhatt,pbhat); 
	  vec_zeros(censXv); vec_zeros(censZv); 

	  totrisk=0; 
	  for (j=0;j<*antpers;j++) { 
	    VE(lrisk,j)=(x[j]>=time); totrisk=totrisk+VE(lrisk,j);
	    extract_row(ldesignX,j,xi); extract_row(ldesignG,j,zi); 

	    lrr=0;  lrrt=0; 
	    // {{{ compute P_1 and DP_1 
	    if (*trans==1) {
	      for (l=0;l<*pg;l++) lrr=lrr+VE(gam,l)*VE(zi,l)*pow(time,timepow[l]); 
	      VE(plamt,j)=1-exp(-VE(pbhat,j)-lrr); 
	      scl_vec_mult(1-VE(plamt,j),xi,xi);
	      scl_vec_mult(1-VE(plamt,j),zi,zi);  
              for (l=0;l<*pg;l++) VE(zi,l)=pow(time,timepow[l])*VE(zi,l); 
              if ((entry[j]>0)) { 
	              extract_row(ldesignG,j,zit); extract_row(ldesignX,j,xit); 
	              for(i=1;i<=*px;i++) VE(truncbhatt,i-1)=cumentry[i*(*antpers)+j];
	              for (l=0;l<*pg;l++) lrrt=lrrt+VE(gam,l)*VE(zit,l)*pow(entry[j],timepow[l]); 
		      phattrunc=1-exp(-vec_prod(xit,truncbhatt)-lrrt); 
	              scl_vec_mult(1/trunkp[j],xi,xi); scl_vec_mult(1/trunkp[j],zi,zi);  
	              VE(plamt,j)=VE(plamt,j)-phattrunc;
            }
	    }
	    if (*trans==2) {  // FG-prop-model
	      for (l=0;l<*pg;l++) lrr=lrr+VE(gam,l)*VE(zi,l)*pow(time,timepow[l]); 
	      VE(rr,j)=exp(lrr);  
	      VE(plamt,j)=1-exp(-exp(VE(pbhat,j))*VE(rr,j)); 
	      scl_vec_mult((1-VE(plamt,j))*exp(VE(pbhat,j))*VE(rr,j),xi,xi); 
	      scl_vec_mult((1-VE(plamt,j))*exp(VE(pbhat,j))*VE(rr,j),zi,zi); 
	      for (l=0;l<*pg;l++) VE(zi,l)= pow(time,timepow[l])*VE(zi,l); 
              if ((entry[j]>0)) { 
	         extract_row(ldesignG,j,zit); extract_row(ldesignX,j,xit); 
	         for(i=1;i<=*px;i++) VE(truncbhatt,i-1)=cumentry[i*(*antpers)+j];
	         for (l=0;l<*pg;l++) lrrt=lrrt+VE(gam,l)*VE(zit,l)*pow(entry[j],timepow[l]); 
	         phattrunc=1-exp(-exp(vec_prod(xit,truncbhatt))*exp(lrrt)); 
	         scl_vec_mult(1/trunkp[j],xi,xi); scl_vec_mult(1/trunkp[j],zi,zi);  
	         VE(plamt,j)=VE(plamt,j)-phattrunc;
	      } 
	   }
           if (*trans==6) { // FG-parametrization 
	      for (l=0;l<*pg;l++) lrr=lrr+VE(gam,l)*VE(zi,l)*pow(time,timepow[l]); 
	      VE(rr,j)=exp(lrr);  
	      VE(plamt,j)=1-exp(-VE(pbhat,j)*VE(rr,j)); 
	      scl_vec_mult((1-VE(plamt,j))*VE(rr,j),xi,xi); 
	      scl_vec_mult((1-VE(plamt,j))*VE(pbhat,j)*VE(rr,j),zi,zi); 
	      for (l=0;l<*pg;l++) VE(zi,l)= pow(time,timepow[l])*VE(zi,l); 
	      if ((entry[j]>0)) { 
	         extract_row(ldesignG,j,zit); extract_row(ldesignX,j,xit); 
	         for(i=1;i<=*px;i++) VE(truncbhatt,i-1)=cumentry[i*(*antpers)+j];
	         for (l=0;l<*pg;l++) lrrt=lrrt+VE(gam,l)*VE(zit,l)*pow(entry[j],timepow[l]); 
	         phattrunc=1-exp(-vec_prod(xit,truncbhatt)*exp(lrrt)); 
	         scl_vec_mult(1/trunkp[j],xi,xi);
	         scl_vec_mult(1/trunkp[j],zi,zi);  
	         VE(plamt,j)=VE(plamt,j)-phattrunc;
	      } 
	    }
            if (*trans==3) { // logistic
	      for (l=0;l<*pg;l++) lrr=lrr+VE(gam,l)*VE(zi,l)*pow(time,timepow[l]); 
	      VE(rr,j)=exp(lrr);  
	      VE(plamt,j)=exp(VE(pbhat,j)+lrr)/(1+exp(VE(pbhat,j)+lrr)); 
	      dummy=VE(plamt,j)/(1+exp(VE(pbhat,j)+lrr)); 
	      scl_vec_mult(dummy,xi,xi); 
	      scl_vec_mult(dummy,zi,zi); 
   	      for (l=0;l<*pg;l++) VE(zi,l)= pow(time,timepow[l])*VE(zi,l); 
	      if ((entry[j]>0)) { 
	         extract_row(ldesignG,j,zit); extract_row(ldesignX,j,xit); 
	         for(i=1;i<=*px;i++) VE(truncbhatt,i-1)=cumentry[i*(*antpers)+j];
	         for (l=0;l<*pg;l++) lrrt=lrrt+VE(gam,l)*VE(zit,l)*pow(entry[j],timepow[l]); 
	         phattrunc= exp(vec_prod(xit,truncbhatt)+lrrt)/(1+exp(vec_prod(xit,truncbhatt)+lrrt)); 
	         scl_vec_mult(1/trunkp[j],xi,xi);
	         scl_vec_mult(1/trunkp[j],zi,zi);  
	         VE(plamt,j)=VE(plamt,j)-phattrunc;
	      } 
           }
           if (*trans==7) { // logistic, baseline direct parametrization
	      for (l=0;l<*pg;l++) lrr=lrr+VE(gam,l)*VE(zi,l)*pow(time,timepow[l]); 
	      VE(rr,j)=exp(lrr);  
	      VE(plamt,j)=VE(pbhat,j)*exp(lrr)/(1+VE(pbhat,j)*exp(lrr)); 
	      dummy=exp(lrr)/pow(1+VE(pbhat,j)*exp(lrr),2); 
	      scl_vec_mult(dummy,xi,xi); 
	      scl_vec_mult(VE(pbhat,j)*dummy,zi,zi); 
   	      for (l=0;l<*pg;l++) VE(zi,l)= pow(time,timepow[l])*VE(zi,l); 
	      if ((entry[j]>0)) { 
	         extract_row(ldesignG,j,zit); extract_row(ldesignX,j,xit); 
	         for(i=1;i<=*px;i++) VE(truncbhatt,i-1)=cumentry[i*(*antpers)+j];
	         for (l=0;l<*pg;l++) lrrt=lrrt+VE(gam,l)*VE(zit,l)*pow(entry[j],timepow[l]); 
	         phattrunc= vec_prod(xit,truncbhatt)*exp(lrrt)/(1+vec_prod(xit,truncbhatt)*exp(lrrt)); 
	         scl_vec_mult(1/trunkp[j],xi,xi);
	         scl_vec_mult(1/trunkp[j],zi,zi);  
	         VE(plamt,j)=VE(plamt,j)-phattrunc;
	      } 
           }
	   if (*trans==4) { // relative risk 
	      for (l=0;l<*pg;l++) VE(zi,l)= pow(time,timepow[l])*VE(zi,l); 
	      for (l=0;l<*pg;l++) lrr=lrr+VE(gam,l)*VE(zi,l); 
	      VE(rr,j)=lrr;  
	      VE(plamt,j)=exp(VE(pbhat,j)+lrr); 
	      scl_vec_mult(VE(plamt,j),xi,xi); 
	      scl_vec_mult(VE(plamt,j),zi,zi); 
	      if ((entry[j]>0)) { 
	         extract_row(ldesignG,j,zit); extract_row(ldesignX,j,xit); 
	         for(i=1;i<=*px;i++) VE(truncbhatt,i-1)=cumentry[i*(*antpers)+j];
	         for (l=0;l<*pg;l++) lrrt=lrrt+VE(gam,l)*VE(zit,l)*pow(entry[j],timepow[l]); 
	         phattrunc= exp(vec_prod(xit,truncbhatt)+exp(lrrt));
	         scl_vec_mult(1/trunkp[j],xi,xi);
	         scl_vec_mult(1/trunkp[j],zi,zi);  
	         VE(plamt,j)=VE(plamt,j)-phattrunc;
	      } 
           }
	   if (*trans==5) { // relative risk, param 2
	      for (l=0;l<*pg;l++) VE(zi,l)= pow(time,timepow[l])*VE(zi,l); 
	      for (l=0;l<*pg;l++) lrr=lrr+VE(gam,l)*VE(zi,l); // *pow(time,timepow[l]); 
	      VE(rr,j)=lrr;  
	      VE(plamt,j)=VE(pbhat,j)*exp(lrr); 
	      scl_vec_mult(exp(lrr),xi,xi); 
	      scl_vec_mult(VE(plamt,j),zi,zi); 
	      if ((entry[j]>0)) { 
	         extract_row(ldesignG,j,zit); extract_row(ldesignX,j,xit); 
	         for(i=1;i<=*px;i++) VE(truncbhatt,i-1)=cumentry[i*(*antpers)+j];
	         for (l=0;l<*pg;l++) lrrt=lrrt+VE(gam,l)*VE(zit,l)*pow(entry[j],timepow[l]); 
	         phattrunc= vec_prod(xit,truncbhatt)*exp(exp(lrrt));
	         scl_vec_mult(1/trunkp[j],xi,xi);
	         scl_vec_mult(1/trunkp[j],zi,zi);  
	         VE(plamt,j)=VE(plamt,j)-phattrunc;
	      } 
	   if (*trans==8) { // log-relative risk,
	      for (l=0;l<*pg;l++) VE(zi,l)= pow(time,timepow[l])*VE(zi,l); 
	      for (l=0;l<*pg;l++) lrr=lrr+VE(gam,l)*VE(zi,l); // *pow(time,timepow[l]); 
	      VE(rr,j)=lrr;  
	      VE(plamt,j)=VE(pbhat,j)*exp(exp(lrr)); 
	      scl_vec_mult(exp(exp(lrr)),xi,xi); 
	      scl_vec_mult(VE(plamt,j)*exp(lrr),zi,zi); 
	      if ((entry[j]>0)) { 
	         extract_row(ldesignG,j,zit); extract_row(ldesignX,j,xit); 
	         for(i=1;i<=*px;i++) VE(truncbhatt,i-1)=cumentry[i*(*antpers)+j];
	         for (l=0;l<*pg;l++) lrrt=lrrt+VE(gam,l)*VE(zit,l)*pow(entry[j],timepow[l]); 
	         phattrunc= vec_prod(xit,truncbhatt)*exp(exp(lrrt));
	         scl_vec_mult(1/trunkp[j],xi,xi);
	         scl_vec_mult(1/trunkp[j],zi,zi);  
	         VE(plamt,j)=VE(plamt,j)-phattrunc;
	      } 
	   }
           }
	   // }}}
	   
	    VE(Y,j)=((x[j]<=time) & (cause[j]==*CA))*1;

           if ((itt==(*Nit-1)) && (*robust==0)) { // {{{ for censoring distribution correction 
              if (KMc[j]>0.001) scl_vec_mult(weights[j]*VE(Y,j)*1/KMc[j],xi,rowX); 
	      else scl_vec_mult(weights[j]*VE(Y,j)*1/0.001,xi,rowX); 
              vec_add(censXv,rowX,censXv); 
              replace_row(censX,j,rowX);
              if (KMc[j]>0.001) scl_vec_mult(weights[j]*VE(Y,j)*1/KMc[j],zi,rowZ); 
	      else scl_vec_mult(weights[j]*VE(Y,j)*1/0.001,zi,rowZ); 
              vec_add(censZv,rowZ,censZv); 
              replace_row(censZ,j,rowZ);
           } // }}}

	   if (*estimator==1) scl_vec_mult(pow(weights[j],0.5)*(time>entry[j]),xi,xi); 
           else scl_vec_mult(pow(weights[j],0.5)*(time< KMc[j])*(time>entry[j]),xi,xi); 
	   if (*estimator==1) scl_vec_mult(pow(weights[j],0.5)*(time>entry[j]),zi,zi); 
           else scl_vec_mult(pow(weights[j],0.5)*(time< KMc[j])*(time>entry[j]),zi,zi); 
	   replace_row(cdesignX,j,xi); replace_row(cdesignG,j,zi); 


	   if (*estimator==1) {
	   if (KMc[j]<0.001) VE(Y,j)=((VE(Y,j)/0.001)-VE(plamt,j)/trunkp[j])*(time>entry[j]); 
	   else VE(Y,j)=((VE(Y,j)/KMc[j])-VE(plamt,j)/trunkp[j])*(time>entry[j]);
	   } else VE(Y,j)=(VE(Y,j)-VE(plamt,j)/trunkp[j])*(time<KMc[j])*(time>entry[j]);;
	   VE(Y,j)=pow(weights[j],0.5)*VE(Y,j); 
	  }

	  MtM(cdesignX,A); 
	  invertS(A,AI,osilent); sing=0; 
          if (ME(AI,0,0)==0 && *stratum==0 && (osilent==0)) {
	     Rprintf(" X'X not invertible at time %d %lf %d \n",s,time,osilent); 
	     print_mat(A); 
          }
          if (*stratum==1)  {
	     for (k=0;k<*px;k++) if (fabs(ME(A,k,k))<0.000001)  ME(AI,k,k)=0; else ME(AI,k,k)=1/ME(A,k,k);
          }


          if (( fabs(ME(AI,0,0))<.0000001) && (*stratum==0)) {
             convproblems=1;  silent[s]=1; 
             if (osilent==0) Rprintf("Iteration %d: non-invertible design at time %lf\n",itt,time); 
	     for (k=1;k<=*px;k++) inc[k*(*Ntimes)+s]=0; 
	     sing=1;
          }

	  if (sing==0) { 
	  vM(cdesignX,Y,xi); Mv(AI,xi,AIXdN); 
	  if (*fixgamma==0) {
	  MtM(cdesignG,ZZ); MtA(cdesignX,cdesignG,XZ);
	  MxA(AI,XZ,XZAI); MtA(XZAI,XZ,tmpM2); 
	  mat_subtr(ZZ,tmpM2,dCGam); 
	  scl_mat_mult(dtime,dCGam,dCGam); mat_add(CGam,dCGam,CGam); 

	  vM(cdesignG,Y,zi); vM(XZ,AIXdN,tmpv2); 
	  vec_subtr(zi,tmpv2,ZGdN); scl_vec_mult(dtime,ZGdN,ZGdN); 
	  vec_add(ZGdN,IZGdN,IZGdN); 
	  Acorb[s]=mat_transp(XZAI,Acorb[s]); 
	  C[s]=mat_copy(XZ,C[s]); 
	  }

	  /* scl_mat_mult(dtime,XZAI,tmpM4);mat_add(tmpM4,Ct,Ct); */
	  for (k=1;k<=*px;k++) inc[k*(*Ntimes)+s]=VE(AIXdN,k-1); 

	  if (itt==(*Nit-1)) 
	  for (i=0;i<*antpers;i++) 
          { // vec_zeros(tmpv1); vec_zeros(z1); 
              j=clusters[i]; 	
	      extract_row(cdesignX,i,xi); scl_vec_mult(VE(Y,i),xi,xi); 
	      Mv(AI,xi,rowX);
	      for (l=0;l<*px;l++) ME(W3t[j],s,l)+=VE(rowX,l); 
              if (*fixgamma==0) {
	         extract_row(cdesignG,i,zi); scl_vec_mult(VE(Y,i),zi,zi); 
	         vM(C[s],rowX,tmpv2); vec_subtr(zi,tmpv2,rowZ); 
	         scl_vec_mult(dtime,rowZ,rowZ); 
	         vec_add(rowZ,W2[j],W2[j]); 
	      }

	      if (*robust==0) { // {{{ censuring terms  
              k=ordertime[i]; nrisk=(*antpers)-i; 
	      clusterj=clusters[k]; 
	      if (cause[k]==(*censcode)) { 
	         Mv(AI,censXv,rowX);
	         for (l=0;l<*px;l++) ME(W3t[clusterj],s,l)+=VE(rowX,l)/nrisk; 
		 if (*fixgamma==0) {
	         vM(C[s],rowX,tmpv2); vec_subtr(censZv,tmpv2,rowZ); 
	         scl_vec_mult(dtime/nrisk,rowZ,rowZ); 
	         for (l=0;l<*pg;l++) VE(W2[clusterj],l)+=VE(rowZ,l)/nrisk; 
	         }
	         for (j=i;j<*antpers;j++) {
	            clusterj=clusters[ordertime[j]]; 	
	            for (l=0;l<*px;l++) ME(W3t[clusterj],s,l)-=VE(rowX,l)/pow(nrisk,2); 
		    if (*fixgamma==0) {
	               for (l=0;l<*pg;l++) VE(W2[clusterj],l)+=VE(rowZ,l)/nrisk; 
//	               scl_vec_mult(-1/(nrisk),rowZ,rowZ); 
//	               vec_add(rowZ,W2[clusterj],W2[clusterj]); 
	            }
	         } 
	      } 
             // fewer where I(s <= T_i) , because s is increasing
             extract_row(censX,k,xi); vec_subtr(censXv,xi,censXv);  
             extract_row(censZ,k,zi); vec_subtr(censZv,zi,censZv);  
           }     // robust==0 }}}

	   } // if (itt==(*Nit-1)) for (i=0;i<*antpers;i++) 

	  } // sing=0

	} /* s=1,...Ntimes */

      dummy=0; 
      if (*fixgamma==0) {
      invertS(CGam,ICGam,osilent); Mv(ICGam,IZGdN,dgam); vec_add(gam,dgam,gam); 

      if (isnan(vec_sum(dgam))) {
         if (convproblems==1) convproblems=3;  else convproblems=2; 
         if (osilent==1) Rprintf("missing values in dgam %ld \n",(long int) s);
	 vec_zeros(gam); 
      }

      for (k=0;k<*pg;k++)  dummy=dummy+fabs(VE(dgam,k)); 
      }

      for (s=0;s<*Ntimes;s++) {
      if (*fixgamma==0) vM(Acorb[s],dgam,korG); 
	est[s]=times[s]; var[s]=times[s]; 
	for (k=1;k<=*px;k++)  { 
            est[k*(*Ntimes)+s]= est[k*(*Ntimes)+s]+inc[k*(*Ntimes)+s]-VE(korG,k-1); 
	  dummy=dummy+fabs(inc[k*(*Ntimes)+s]-VE(korG,k-1)); 
	  /* Rprintf(" %lf ",est[k*(*Ntimes)+s]); Rprintf(" \n");*/ }
      } /* s=1,...Ntimes */
      if (dummy<*convc && itt<*Nit-2) itt=*Nit-2; 

      if (*detail==1) { 
	Rprintf(" iteration %d %d \n",itt,*Nit); 
	Rprintf("Total sum of changes %lf \n",dummy); 
	Rprintf("Gamma parameters \n"); print_vec(gam); 
	Rprintf("Change in Gamma \n"); print_vec(dgam); 
	Rprintf("===========================================================\n"); 
      }

    } /*itt l�kke */ 

   R_CheckUserInterrupt();
  /* ROBUST VARIANCES   */ 
//  if (*robust==1) 
//    {
      for (s=0;s<*Ntimes;s++) {
	vec_zeros(VdB); 
	 for (i=0;i<*antclust;i++) {
          if (*fixgamma==0) { 
	     Mv(ICGam,W2[i],tmpv2); vM(Acorb[s],tmpv2,rowX); 
	  } else vec_zeros(rowX); 
	  extract_row(W3t[i],s,tmpv1); vec_subtr(tmpv1,rowX,difX); 
	  replace_row(W4t[i],s,difX); vec_star(difX,difX,tmpv1); 
	  vec_add(tmpv1,VdB,VdB);

	  if (*resample==1) {
	  if ((s==1) & (*fixgamma==0)) for (c=0;c<*pg;c++) gamiid[c*(*antclust)+i]=gamiid[c*(*antclust)+i]+VE(tmpv2,c);
	    for (c=0;c<*px;c++) {l=i*(*px)+c; biid[l*(*Ntimes)+s]=biid[l*(*Ntimes)+s]+VE(difX,c);} 
	  }

          if (*fixgamma==0)  if (s==0) { for (j=0;j<*pg;j++) for (k=0;k<*pg;k++) 
			ME(RobVargam,j,k)=ME(RobVargam,j,k)+VE(tmpv2,j)*VE(tmpv2,k);} 
	}  /* for (i=0;i<*antclust;i++) */ 
	for (k=1;k<*px+1;k++) var[k*(*Ntimes)+s]=VE(VdB,k-1); 

      } /* s=0..Ntimes*/
//    }

  /* MxA(RobVargam,ICGam,tmpM2); MxA(ICGam,tmpM2,RobVargam);*/
  /* print_mat(RobVargam);  */ 

  if (*fixgamma==0) { 
     for (j=0;j<*pg;j++) {gamma[j]=VE(gam,j);
       for (k=0;k<*pg;k++) {vargamma[k*(*pg)+j]=ME(RobVargam,j,k);}}
  }

  if (convproblems>=1) convc[0]=convproblems; 
   R_CheckUserInterrupt();
  if (*sim==1) {
    comptestfunc(times,Ntimes,px,est,var,vcudif,antsim,test,testOBS,Ut,simUt,W4t,weighted,antclust,gamma2,line,timepowtest);
  }

  // {{{ freeing
  free_mats(&censX,&censZ,&ldesignX,&A,&AI,&cdesignX,&ldesignG,&cdesignG,
	      &S,&dCGam,&CGam,&ICGam,&VarKorG,&dC,&XZ,&ZZ,&ZZI,&XZAI, 
	      &Ct,&tmpM1,&tmpM2,&tmpM3,&tmpM4,&Vargam,&dVargam,
	      &Delta,&dM1M2,&M1M2t,&RobVargam,NULL); 

  free_vecs(&censXv,&censZv,&qs,&Y,&rr,&bhatub,&diag,&dB,&dN,&VdB,&AIXdN,&AIXlamt,
	      &bhatt,&pbhat,&plamt,&korG,&pghat,&rowG,&gam,&dgam,&ZGdN,&IZGdN,
	      &ZGlamt,&IZGlamt,&xit,&xi,&rowX,&rowZ,&difX,&zit,&zi,&z1,&tmpv1,&tmpv2,&lrisk,&ciftrunk,&truncbhatt,
	      NULL); 

  for (j=0;j<*Ntimes;j++) {free_mat(Acorb[j]);free_mat(C[j]);free_mat(M1M2[j]);}
  for (j=0;j<*antclust;j++) {free_mat(W3t[j]); free_mat(W4t[j]);
    free_vec(W2[j]); free_vec(W3[j]); }
  free(vcudif); free(inc); free(n); free(nx);  free(cumentry); free(px1); 
  free(cifentry); 
  // }}}
}

double mypow(double x,double p)
{
  double val,log(),exp(); 
  val=exp(log(x)*p); 
  return(val); 
}
