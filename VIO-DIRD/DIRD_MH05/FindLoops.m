function loop=FindLoops(threshold,results)
loop=[];
for i=1:size(results,1)
    if results(i,3)>=threshold
        loop=[loop;results(i,1:2)];
    end
end
end