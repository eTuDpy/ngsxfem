
# load geometry
geometry = d7_stokes.in2d                                        
# and mesh
mesh = d7_stokes.vol.gz

#load xfem-library and python-bindings
shared = libngsxfem_xfem                                       
shared = libngsxfem_xstokes                                       
# pymodule = d7_stokes

define constant heapsize = 1e9

define constant R = 0.3333
define constant one = 1.0

# interface description as zero-level
# define coefficient lset
# ( sqrt(x*x+y*y) - R),

define coefficient lset
( sqrt((x)*(x)+y*y) - R),

define fespace fescomp
       -type=xstokes
       -order=1                 
       -dirichlet_vel=[1,2,3,4]
       -empty_vel
       # -dgjumps
       -ref_space=1

numproc informxstokes npi_px 
        -xstokesfespace=fescomp
        -coef_levelset=lset

define gridfunction uvp -fespace=fescomp

define constant zero = 0.0
define constant one = 1.0
#define constant none = -1.0
define constant lambda = 1000.0
define constant delta = 1.0

define coefficient s
0,1,0,0,

define coefficient gammaf
1.0,


#numproc setvaluesx npsvx -gridfunction=uvp.2 -coefficient_neg=s -coefficient_pos=s -boundary

# integration on sub domains
define linearform f -fespace=fescomp
xsource one zero -comp=2
xGammaForce gammaf
#xLBmeancurv one # naiv Laplace-Beltrami discretization 
# xmodLBmeancurv one lset # improved Laplace-Beltrami discretization 
# integration on sub domains
define bilinearform a -fespace=fescomp -symmetric -linearform=f -printelmat
xstokes one one 
# xlaplace one one -comp=1
# xnitsche one one one one lambda -comp=1
# xnitsche one one one one lambda -comp=2
# lo_ghostpenalty one one delta -comp=1
# lo_ghostpenalty one one delta -comp=2
# xmass one one -comp=1
# xmass 1.0 1.0 -comp=2

#define preconditioner c -type=local -bilinearform=a -test #-block           
define preconditioner c -type=direct -bilinearform=a -inverse=pardiso #-test 

numproc bvp npbvp -gridfunction=uvp -bilinearform=a -linearform=f -solver=cg -preconditioner=c -maxsteps=1000 -prec=1e-6


define coefficient velocity ( (uvp.1, uvp.2) )
numproc draw npd1 -coefficient=velocity -label=velocity

define coefficient pressure ( (uvp.3) )
numproc draw npd2 -coefficient=pressure -label=pressure

numproc draw npd3 -coefficient=lset -label=levelset

numproc visualization npviz 
        -scalarfunction=levelset
        # -vectorfunction=velocity
        -minval=0 -maxval=0 
        -nolineartexture -deformationscale=1 -subdivision=3
