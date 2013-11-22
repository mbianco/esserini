function plotdata(LEN)
%while(1)
load statdata.dat
figure(1);hold off;
range = [max(1,length(statdata)-LEN):length(statdata)];
plot(range,statdata(range,1)); 
hold on; 
plot(range,statdata(range,5),'r');
plot(range,statdata(range,2),'g');
plot(range,statdata(range,3),'k'); 
plot(range,statdata(range,16),'m'); 
title('esserini'); legend ('beings', 'weight', 'food', 'manure', 'avg age','Location','NorthWest')
figure(2);hold off;plot(range,statdata(range,3)+statdata(range,2)+statdata(range,4)+statdata(range,5)); hold on; plot(range,statdata(range,8),'r')
figure(3); hold off; 
borns=statdata(2:length(statdata),9)-statdata(1:length(statdata)-1,9);
deaths=statdata(2:length(statdata),10)-statdata(1:length(statdata)-1,10);
nat_deaths=statdata(2:length(statdata),11)-statdata(1:length(statdata)-1,11);
fights=statdata(2:length(statdata),12)-statdata(1:length(statdata)-1,12);
nnatdeaths=deaths-nat_deaths;
learn=statdata(:,17)./statdata(:,1);%-statdata(1:length(statdata)-1,17);
range=[max(1,length(borns)-LEN):length(borns)];
subplot(5,1,1);plot(range,borns(range));title('borns');
subplot(5,1,2);plot(range,nnatdeaths(range),'r'); title('non natural deaths');
subplot(5,1,3);plot(range,nat_deaths(range),'g'); title('natural deaths');
subplot(5,1,4);plot(range,fights(range),'k'); title('fights');
subplot(5,1,5);plot(range,learn(range),'k'); title('learnt instructions');
pause(10)
%end
