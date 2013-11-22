function [x]=myplotstat(filename)
statdata=fscanfMat( filename );
figure(1);mtlb_hold( 'off');
plot(statdata(:,1)); 
mtlb_hold( 'on'); 
plot(statdata(:,5),'r');
plot(statdata(:,2),'g');
plot(statdata(:,3),'k'); 
plot(statdata(:,4),'c'); 
plot(statdata(:,16),'m'); 
title('esserini'); legend ('beings', 'weight', 'food', 'manure', 'energy', 'avg age','Location','NorthWest', 2)
figure(2);mtlb_hold( 'off');plot(statdata(:,3)+statdata(:,2)+statdata(:,4)+statdata(:,5)); mtlb_hold( 'on'); plot(statdata(:,8),'r')
figure(3); mtlb_hold( 'off'); 
borns=statdata(2:size(statdata,1),9)-statdata(1:size(statdata,1)-1,9);
deaths=statdata(2:size(statdata,1),10)-statdata(1:size(statdata,1)-1,10);
nat_deaths=statdata(2:size(statdata,1),11)-statdata(1:size(statdata,1)-1,11);
subplot(3,1,1);plot(borns);title('borns');
subplot(3,1,2);plot(deaths-nat_deaths,'r'); title('non natural deaths');
subplot(3,1,3);plot(nat_deaths,'g'); title('natural deaths');
x=1
endfunction

