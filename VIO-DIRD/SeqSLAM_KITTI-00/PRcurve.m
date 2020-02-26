load('TrueLoop.mat');
load('data.mat');
load('results.mat');
for thresh=0.5:0.01:1
    y=0;
    n=0;
     m = results.matches(:,1);
     m(results.matches(:,2)>thresh) = NaN;
      for i=1:length(m)
         if i>=m(i)
        m(i)=NaN;
         end
       end
     for i=1:length(m)
     if isnan(m(i))
            y=y+1;  
     end
     end
    for i=1:length(m)
        for j=1:size(trueloop,1)
         if trueloop(j,1)==i&&trueloop(j,2)==m(i)
            n=n+1;
         end
        end
    end
    p=n/(length(m)-y);
    r=n/size(trueloop,1);    
    plot(r,p,'*')
    hold on
end
        