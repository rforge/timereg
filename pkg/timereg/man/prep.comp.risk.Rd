\name{prep.comp.risk}
\alias{prep.comp.risk}
\title{Set up weights for delayed-entry competing risks data for comp.risk function}
\description{
Computes the weights of Geskus (2011) modified to the setting of the comp.risk function.
The returned weights are 
\eqn{1/(H(T_i)*G_c(min(T_i,tau)))} and tau is the max of the times argument, 
here \eqn{H} is the estimator of the truncation distribution and \eqn{G_c} is the right censoring distribution.
}
\usage{
prep.comp.risk(data,times=NULL,entrytime=NULL,
	       time="time",cause="cause",cname="cweight",tname="tweight",
	       strata=NULL,nocens.out=TRUE,cens.formula=NULL,cens.code=0,
	       prec.factor=100)
}
\arguments{
\item{data}{ data frame for comp.risk.}
\item{times}{ times for estimating equations.}
\item{entrytime}{name of delayed entry variable, if not given computes right-censoring 
case.}
\item{time}{name of survival time variable.}
\item{cause}{name of cause indicator}
\item{cname}{name of censoring weight.}
\item{tname}{name of truncation weight.}
\item{strata}{strata variable to obtain stratified weights.}
\item{nocens.out}{returns only uncensored part of data-frame}
\item{cens.formula}{censoring model formula for Cox models for the truncation and censoring 
model.}
\item{cens.code}{code for censoring among causes.}
\item{prec.factor}{precision factor, for ties between censoring/even times, truncation times/event times}
}
\value{
Returns an object. With the following arguments:
\item{dataw}{a data.frame with weights.}

The function wants to make two new variables "weights" and "cw" so if these already are in
the data frame it tries to add an "_" in the names. 
}
\references{
Geskus (2011), Cause-Specific Cumulative Incidence Estimation and the Fine and Gray Model Under
Both Left Truncation and Right Censoring, Biometrics (2011), pp 39-49.  

Shen (2011), Proportional subdistribution hazards regression for left-truncated competing 
risks data, Journal of Nonparametric Statistics (2011), 23, 885-895
}
\author{Thomas Scheike}
\examples{
data(bmt)
nn <- nrow(bmt)
entrytime <- rbinom(nn,1,0.5)*(bmt$time*runif(nn))
bmt$entrytime <- entrytime
times <- seq(5,70,by=1)

### adds weights to uncensored observations
bmtw <- prep.comp.risk(bmt,times=times,time="time",
		       entrytime="entrytime",cause="cause")

#########################################
### nonparametric estimates
#########################################
## {{{ 
### nonparametric estimates, right-censoring only 
out <- comp.risk(Event(time,cause)~+1,data=bmt,
		 cause=1,model="rcif2",
		 times=c(5,30,70),n.sim=0)
out$cum
### same as 
###out <- prodlim(Hist(time,cause)~+1,data=bmt)
###summary(out,cause="1",times=c(5,30,70))

### with truncation 
out <- comp.risk(Event(time,cause)~+1,data=bmtw,cause=1,
  model="rcif2",
  cens.weight=bmtw$cw,weights=bmtw$weights,times=c(5,30,70),
  n.sim=0)
out$cum
### same as
###out <- prodlim(Hist(entry=entrytime,time,cause)~+1,data=bmt)
###summary(out,cause="1",times=c(5,30,70))
## }}} 

#########################################
### Regression 
#########################################
## {{{ 
### with truncation correction
out <- comp.risk(Event(time,cause)~const(tcell)+const(platelet),data=bmtw,
 cause=1,cens.weight=bmtw$cw,
 weights=bmtw$weights,times=times,n.sim=0)
summary(out)

### with only righ-censoring, standard call
outn <- comp.risk(Event(time,cause)~const(tcell)+const(platelet),data=bmt,
	  cause=1,times=times,n.sim=0)
summary(outn)
## }}} 

}
\keyword{survival}
