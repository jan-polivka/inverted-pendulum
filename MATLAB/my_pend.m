clc; clear; close all;

%%
M = 0.1;
m = 0.25;
bf = 0.1;
l = 0.1;
I = 0.005;
g = 9.81;

q = I*(M+m) + M*m*l^2;

A = [0 1 0 0;
    0 -bf*I*(M+m)/q (m^2)*(l^2)*g/q 0;
    0 0 0 1;
    0 -bf*m*l/q m*g*l*(M+m)/q 0];

B = [0; (I+m*l^2)/q; 0; m*l/q];

C = [1 0 0 0;
    0 0 1 0];

D = [0; 0];

%%
Mo = [C; C*A; C*A^2; C*A^3];
Mc = [B A*B (A^2)*B (A^3)*B];
MoRank = rank(Mo); %Observable
McRank = rank(Mc); %Controllable

sys = ss(A,B,C,D);

stable = isstable(sys);

step(sys)
figure
impulse(sys)



%%
% LQR process
close all;
R = 0.1;
Q = [40 0 0 0;
     0 0 0 0;
     0 0 1 0;
     0 0 0 0];

[K, S, P] = lqr(sys, Q, R);
Ky = K;
sysStab = sys;
sysStab.A = sys.A - sys.B*K;

step(sysStab)
Si = stepinfo(sysStab);
Si(1).RiseTime
Si(2).RiseTime


%%
% Observer process using place

pps = pole(sysStab)*10;
L = transpose(place(transpose(sys.A), transpose(sys.C), pps));
sysOb = sys;
sysOb.A = sys.A - L*sys.C;

close all;
step(sysOb)
hold on;
step(sysStab)
legend

%%
% Combination

s = size(A,1);
Z = [zeros([1,s]) 1];
N = inv([A,B;[1 0 0 0],0])*Z';
Nx = N(1:s);
Nu = N(1+s);
Nbar=Nu + K*Nx;

Ac = [A-B*K B*K; zeros(size(A)) A-L*C];
Bc = [B; zeros(size(B))];
Cc = [C zeros(size(C))];
Dc = [0;0];

sysC = ss(Ac,Bc,Cc,Dc);

Sc = stepinfo(sysC);

dc = dcgain(sysC);
Bcdc = Bc*(1/dc);
Bcdc = Bcdc(:,1);

sysDC = ss(Ac,Bcdc,Cc,Dc);
Sdc = stepinfo(sysDC);
%%
close all;
figure
step(sysDC)
hold on;
step(sysC)
%%
% [b,a] = ss2tf(Ac,Bcdc,Cc,Dc);
% b = {b(1,:);b(2,:)};
% 
% Hm = tf(b,a)
% %%
% b1 = b(1,:);
% b2 = b(2,:);
% 
% Hm1 = tf(b1,a)
% Hm2 = tf(b2,a)
%%
[yC,tC,xC] = step(sysC);
sysC_dis = c2d(sysC, 0.001);
[yDis, tDis, xDis] = step(sysC_dis);

