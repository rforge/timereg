prop<-function(x) x

two.stage<-function(margsurv,data=sys.parent(),
Nit=60,detail=0,start.time=0,max.time=NULL,id=NULL,clusters=NULL,
robust=1,theta=NULL,theta.des=NULL,var.link=0,step=1,notaylor=0)
{ ## {{{
## {{{ seting up design and variables
 rate.sim <- 1; 
 formula<-attr(margsurv,"Formula");
 beta.fixed <- attr(margsurv,"beta.fixed")
 if (is.null(beta.fixed)) beta.fixed <- 1; 
 ldata<-aalen.des(formula,data=data,model="cox.aalen");
 id <- attr(margsurv,"id"); clusters <- attr(margsurv,"cluster")
 X<-ldata$X; time<-ldata$time2; Z<-ldata$Z;  status<-ldata$status;
 time2 <- attr(margsurv,"time")
 start <- attr(margsurv,"start")
 antpers<-nrow(X);
  if (is.null(Z)==TRUE) {npar<-TRUE; semi<-0;}  else {
		 Z<-as.matrix(Z); npar<-FALSE; semi<-1;}
  if (npar==TRUE) {Z<-matrix(0,antpers,1); pz<-1; fixed<-0;} else {fixed<-1;pz<-ncol(Z);}
  px<-ncol(X);
  antclust <- length(unique(clusters))

  if (attr(margsurv,"residuals")!=2) stop("residuals=2  for  marginal model\n"); 
  residuals <- margsurv$residuals$dM

  Biid<-c(); gamma.iid <- 0; 
  if (notaylor==0) {
    if (!is.null(margsurv$B.iid))
    for (i in 1:antclust) Biid<-cbind(Biid,margsurv$B.iid[[i]]); 
    if (!is.null(margsurv$gamma.iid)) gamma.iid<-margsurv$gamma.iid;
    if ((is.null(margsurv$B.iid))||(is.null(margsurv$gamma.iid)))cat("missing iid for marginal model\n")
  }

  ratesim<-rate.sim; inverse<-var.link
  pxz <- px + pz;

  times<-c(start.time,time2[status==1]); times<-sort(times);
  if (is.null(max.time)==TRUE) maxtimes<-max(times)+0.1 else maxtimes<-max.time; 
  times<-times[times<maxtimes]
  Ntimes <- sum(status); 

    if (is.null(theta.des)==TRUE) ptheta<-1; 
    if (is.null(theta.des)==TRUE) theta.des<-matrix(1,antpers,ptheta) else
    theta.des<-as.matrix(theta.des); 
    ptheta<-ncol(theta.des); 
    if (nrow(theta.des)!=antpers) stop("Theta design does not have correct dim");

    if (is.null(theta)==TRUE) theta<-rep(0.1,ptheta); 
    if (length(theta)!=ptheta) theta<-rep(theta[1],ptheta); 
    theta.score<-rep(0,ptheta);Stheta<-var.theta<-matrix(0,ptheta,ptheta); 

    cluster.size<-as.vector(table(clusters));
    maxclust<-max(cluster.size)
    idiclust<-matrix(0,antclust,maxclust); 
    cs<- rep(1,antclust)
    for (i in 1:antpers) { 
        idiclust[clusters[i]+1,cs[clusters[i]+1]]<-i-1;
        cs[clusters[i]+1]<- cs[clusters[i]+1]+1; 
    } 
    if (maxclust==1) stop("No clusters !, maxclust size=1\n"); 
  ## }}}

###    dyn.load("two-stage-reg.so"); 

    nparout <- .C("twostagereg", 
        as.double(times), as.integer(Ntimes), as.double(X),
       	as.integer(antpers), as.integer(px), as.double(Z), 
	as.integer(antpers), as.integer(pz), as.integer(antpers),
	as.double(start),as.double(time2), as.integer(Nit), 
	as.integer(detail), as.integer(id), as.integer(status), 
	as.integer(ratesim), as.integer(robust), as.integer(clusters), 
	as.integer(antclust), as.integer(beta.fixed), as.double(theta),
	as.double(var.theta), as.double(theta.score), as.integer(inverse), 
	as.integer(cluster.size), 
	as.double(theta.des), as.integer(ptheta), as.double(Stheta),
	as.double(step), as.integer(idiclust), as.integer(notaylor),
	as.double(gamma.iid),as.double(Biid),as.integer(semi), as.double(residuals)
        ,PACKAGE = "timereg")

## {{{ handling output
   gamma <- margsurv$gamma
   Varbeta <- margsurv$var.gamma; RVarbeta <- margsurv$robvar.gamma;
   score <- margsurv$score; Iinv <- margsurv$D2linv;
   cumint <- margsurv$cum; vcum <- margsurv$var.cum; Rvcu <- margsurv$robvar.cum;

   theta<-matrix(nparout[[21]],ptheta,1);  
   var.theta<-matrix(nparout[[22]],ptheta,ptheta); 
   theta.score<-nparout[[23]]; 
   Stheta<-matrix(nparout[[24]],ptheta,ptheta); 

   ud <- list(cum = cumint, var.cum = vcum, robvar.cum = Rvcu, 
       gamma = gamma, var.gamma = Varbeta, robvar.gamma = RVarbeta, 
       D2linv = Iinv, score = score,  theta=theta,var.theta=var.theta,
       S.theta=Stheta,theta.score=theta.score)

  ptheta<-length(ud$theta); 
  if (ptheta>1) {
                rownames(ud$theta)<-colnames(theta.des);
                names(ud$theta.score)<-colnames(theta.des); } else { 
		names(ud$theta.score)<- rownames(ud$theta)<-"intercept" } 

  attr(ud,"Call")<-sys.call(); 
  class(ud)<-"two.stage"
  attr(ud,"Formula")<-formula;
  attr(ud,"id")<-id;
  attr(ud,"cluster")<-cluster;
  attr(ud,"start")<-start; 
  attr(ud,"time2")<-time2; 
  attr(ud,"var.link")<-var.link
  attr(ud,"beta.fixed")<-beta.fixed

  return(ud) 
  ## }}}

} ## }}}

summary.two.stage<-function (object,digits = 3,...) { ## {{{
  if (!(inherits(object, 'two.stage') )) stop("Must be a Two-Stage object")
  
  prop<-TRUE; 
  if (is.null(object$prop.odds)==TRUE) p.o<-FALSE else p.o<-TRUE
    
  var.link<-attr(object,"var.link");
  cat("Dependence parameter for Clayton-Oakes-Glidden  model\n"); 

  if (sum(abs(object$theta.score)>0.000001) ) 
    cat("Variance parameters did not converge, allow more iterations\n\n"); 

  ptheta<-nrow(object$theta)
  sdtheta<-diag(object$var.theta)^.5
  if (var.link==0) {
      vari<-object$theta
      sdvar<-diag(object$var.theta)^.5
  }
  else {
      vari<-exp(object$theta)
      sdvar<-vari*diag(object$var.theta)^.5
  }
  dep<-cbind(object$theta[,1],sdtheta)
  walddep<-object$theta[,1]/sdtheta; 
  waldpdep<-(1-pnorm(abs(walddep)))*2

  kendall<-1/(1+2/vari) 
  kendall.ll<-1/(1+2/(object$theta+1.96*sdvar)) 
  kendall.ul<-1/(1+2/(object$theta-1.96*sdvar)) 
  if (var.link==0) resdep<-signif(as.matrix(cbind(dep,walddep,waldpdep,kendall)),digits)
  else resdep<-signif(as.matrix(cbind(dep,walddep,waldpdep,vari,sdvar,kendall)),digits);

  if (var.link==0) colnames(resdep) <- c("Variance","SE","z","P-val","Kendall's tau") 
  else colnames(resdep)<-c("log(Variance)","SE","z","P-val","Variance","SE Var.",
                           "Kendall's tau")
  prmatrix(resdep); cat("   \n");  

  if (attr(object,"beta.fixed")==0) {
  cat("Marginal Cox-Aalen model fit\n\n"); 
  if (sum(abs(object$score)>0.000001) && sum(object$gamma)!=0) 
    cat("Marginal model did not converge, allow more iterations\n\n"); 

  if (prop) {
    if (p.o==FALSE) cat("Proportional Cox terms :  \n") else  cat("Covariate effects \n")

    coef.two.stage(object,digits=digits);
  }
  }
   cat("   \n");  cat("  Call: \n"); dput(attr(object, "Call")); cat("\n");
} ## }}}


print.two.stage <- function (x,digits = 3,...) { ## {{{
  if (!(inherits(x, 'two.stage') )) stop("Must be a Two-Stage object")
  cat(" Two-stage estimation for Clayton-Oakes-Glidden  model\n"); 
  cat(" Marginals of Cox-Aalen form, dependence by variance of Gamma distribution\n\n");  
  object <- x; rm(x);
  
  cat(" Nonparametric components : "); 
  cat(colnames(object$cum)[-1]); cat("   \n");  
  if (!is.null(object$gamma)) {
    cat(" Parametric components :  "); cat(rownames(object$gamma)); 
    cat("   \n");
  } 
  cat("   \n");  

  cat(" Call: \n");
  print(attr(object,'Call'))
} ## }}}


coef.two.stage<-function(object,digits=3,d2logl=1,...) {
   coefBase(object,digits=digits,d2logl=d2logl,...)
}

plot.two.stage<-function(x,pointwise.ci=1,robust=0,specific.comps=FALSE,
		level=0.05, 
		start.time=0,stop.time=0,add.to.plot=FALSE,mains=TRUE,
                xlab="Time",ylab ="Cumulative regression function",...) 
{ ## {{{
  if (!(inherits(x, 'two.stage'))) stop("Must be a Two-Stage object")
  object <- x; rm(x);  
 
  B<-object$cum; V<-object$var.cum; p<-dim(B)[[2]]; 
  if (robust>=1) V<-object$robvar.cum; 

  if (sum(specific.comps)==FALSE) comp<-2:p else comp<-specific.comps+1
  if (stop.time==0) stop.time<-max(B[,1]);

  med<-B[,1]<=stop.time & B[,1]>=start.time
  B<-B[med,]; Bs<-B[1,];  B<-t(t(B)-Bs); B[,1]<-B[,1]+Bs[1];
  V<-V[med,]; Vs<-V[1,]; V<-t( t(V)-Vs); 
  Vrob<-object$robvar.cum; 
  Vrob<-Vrob[med,]; Vrobs<-Vrob[1,]; Vrob<-t( t(Vrob)-Vrobs); 

  c.alpha<- qnorm(1-level/2)
  for (v in comp) { 
    c.alpha<- qnorm(1-level/2)
    est<-B[,v];ul<-B[,v]+c.alpha*V[,v]^.5;nl<-B[,v]-c.alpha*V[,v]^.5;
    if (add.to.plot==FALSE) 
      {
        plot(B[,1],est,ylim=1.05*range(ul,nl),type="s",xlab=xlab,ylab=ylab) 
        if (mains==TRUE) title(main=colnames(B)[v]); }
    else lines(B[,1],est,type="s"); 
    if (pointwise.ci>=1) {
      lines(B[,1],ul,lty=pointwise.ci,type="s");
      lines(B[,1],nl,lty=pointwise.ci,type="s"); }
    if (robust>=1) {
      lines(B[,1],ul,lty=robust,type="s"); 
      lines(B[,1],nl,lty=robust,type="s"); }
    abline(h=0); 
  }
}  ## }}}

