% load('data_MH05difficult.mat');
% load('time_match_image_tReal');
load('traj_estimate_true_mh05.mat');
%groundtruth=trajReal.x(:,idx_match_image_tReal(:,2))';
groundtruth_temp=traj_Real_msckf';
groundtruth=groundtruth_temp(401:2229,:);
limita=0.2;
dist=0;
idx=0;
trueloop=[];
for i_m=1:1:size(groundtruth,1)
    plot3(groundtruth(i_m,1),groundtruth(i_m,2),groundtruth(i_m,3),'g.');
    pause(0.01);
    hold on;
    if i_m<10  %不和最近的10帧比较
        continue;
    end
    [idx,dist]=knnsearch(groundtruth(1:i_m-10,:),groundtruth(i_m,:),'k',1);
    if dist<limita
        if abs(idx-i_m)<20  %说明车没有动
            continue;
        else
          trueloop=[trueloop;[idx,i_m]]; 
          plot3(groundtruth(i_m,1),groundtruth(i_m,2),groundtruth(i_m,3),'r*');
          
        end
        
    end
end