\name{restricted.residual.mean}
\alias{restricted.residual.mean}
\title{Estimates restricted residual mean for Cox or Aalen model}
\description{
The restricted means are the 
\deqn{ \int_0^\tau S(t) dt 
}
the standard errors are computed using the i.i.d. decompositions from the
cox.aalen (that must be called with the argument "max.timpoint.sim=NULL")
or aalen function.
}
\usage{
restricted.residual.mean(out,x=0,tau=10,iid=0)
}
\arguments{
\item{out}{ an "cox.aalen" with a Cox model or an "aalen" model.  }
\item{x}{matrix with covariates for Cox model or additive hazards model (aalen).}
\item{tau}{restricted residual mean.}
\item{iid}{if iid=1 then uses iid decomposition for estimation of standard errors.}
}
\details{
must have computed iid decomposition of survival models for standard errors to be 
computed.
Note that competing risks models can be fitted but then the interpretation is not clear. 
}
\value{
Returns an object. With the following arguments:
\item{mean}{restricted mean for different covariates.}
\item{var.mean}{variance matrix.}
\item{se}{standard errors.}
\item{S0tau}{estimated survival functions on time-range [0,tau].}
\item{timetau}{vector of time arguments for S0tau.}
}
\references{
   D. M. Zucker, Restricted mean life with covariates: Modification and extension of a 
   useful survival analysis method, J. Amer. Statist. Assoc. vol. 93 pp. 702-709, 1998.

  Martinussen and Scheike, Dynamic Regression Models for Survival Data, Springer (2006).  
}
\author{Thomas Scheike}
\examples{
### these examples run a bit slowly and a therfore dont-test
\donttest{
data(sTRACE)
sTRACE$cage <- scale(sTRACE$age)
# Fits Cox model  and aalen model 
out<-cox.aalen(Surv(time,status>=1)~prop(cage)+prop(sex)+prop(diabetes)+prop(chf)+
	       prop(vf),data=sTRACE,max.timepoint.sim=NULL,resample.iid=1)
outa<-aalen(Surv(time,status>=1)~cage+sex+diabetes+chf+vf,
data=sTRACE,resample.iid=1)

coxrm <- restricted.residual.mean(out,tau=7,
   x=rbind(c(0,0,0,0,0),c(0,0,0,1,0),c(0,0,0,1,1),c(0,0,0,0,1)),iid=1)
plot(coxrm)
summary(coxrm)

aalenrm <- restricted.residual.mean(outa,tau=7,
   x=rbind(c(1,0,0,0,0,0),c(1,0,0,0,1,0),c(1,0,0,0,1,1),c(1,0,0,0,0,1)),iid=1)
with(aalenrm,matlines(timetau,S0tau,type="s",ylim=c(0,1)))
legend("bottomleft",c("baseline","+chf","+chf+vf","+vf"),
       col=1:4,lty=1)
summary(aalenrm)

mm <-cbind(coxrm$mean,coxrm$se,aalenrm$mean,aalenrm$se)
colnames(mm)<-c("cox-res-mean","se","aalen-res-mean","se")
rownames(mm)<-c("baseline","+chf","+chf+vf","+vf")
mm
}
}
\keyword{survival}
