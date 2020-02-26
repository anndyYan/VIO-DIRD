%% display groundtruth of KITTI poses
% include sequence from 00-10.txt
clear;
idx=0;  %�ȽϺ����֡��ϵ��
dist=0; %�������
limita=2; %����
loop=[];
% read from poses
filename = '00.txt';
fid = fopen(filename);
fseek(fid, 0, 'bof');
lastposition = ftell(fid);
disp(['start position:',num2str(lastposition)]);

groundtruth = [];

while fgetl(fid) ~= -1 % end of line check

    fseek(fid, lastposition, 'bof');
    line = textscan(fid,'%f %f %f %f %f %f %f %f %f %f %f %f\n',1);
    line = [line{:}];
    transform = vec2mat(line,4);

    groundtruth = [groundtruth; [transform(1,4), transform(3,4)]];
    lastposition = ftell(fid);
    disp(['lastposition:',num2str(lastposition)]);

end

numFrame=size(groundtruth,1);
%--------------------------------
%�ж������
figure;
for i_m=1:1:numFrame
    plot(groundtruth(i_m,1),groundtruth(i_m,2),'g.');
    pause(0.01);
    hold on;
    if i_m<10  %���������10֡�Ƚ�
        continue;
    end
    [idx,dist]=knnsearch(groundtruth(1:i_m-10,:),groundtruth(i_m,:),'k',1);
    if dist<limita
        if abs(idx-i_m)<20  %˵����û�ж�
            continue;
        else
          loop=[loop;[idx,i_m]]; 
          plot(groundtruth(i_m,1),groundtruth(i_m,2),'r*');
          
        end
        
    end
end
% figure;
%  plot(groundtruth(:,1),groundtruth(:,2),'ro');
%  hold on ;
%  plot(groundtruth(loop(:,1),1),groundtruth(loop(:,1),2),'g*');
% display ground truth
% %scatter(groundtruth(:,1),groundtruth(:,2));
% figure;
% for i=1:1:numFrame
%   hold on
%   plot(groundtruth(i,1),groundtruth(i,2),'ro');  
%   pause(0.01);
% end

fclose(fid);