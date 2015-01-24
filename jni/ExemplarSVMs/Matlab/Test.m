function Test(filename, colorType)
close all;

Img = imread(filename);
ImgHSV = rgb2hsv(Img);

rData = double(reshape(ImgHSV(:,:,1), [size(Img,1)*size(Img,2) 1]));
gData = double(reshape(ImgHSV(:,:,2), [size(Img,1)*size(Img,2) 1]));
bData = double(reshape(ImgHSV(:,:,3), [size(Img,1)*size(Img,2) 1]));

data = [rData'; gData'; bData'];

load('skin_model.mat');
skin_model = model;

if (strcmp(colorType,'red'))
    load('red_model.mat');
    model.threshold = 50;
elseif (strcmp(colorType,'green'))
    load('green_model.mat');
    model.threshold = 10; 
elseif (strcmp(colorType,'blue'))
    load('blue_model.mat');
    model.threshold = 10;     
elseif (strcmp(colorType,'yellow'))
    load('yellow_model.mat');
    model.threshold = 50;    
elseif (strcmp(colorType,'black'))
    load('black_model.mat');
    model.threshold = 30;   
elseif (strcmp(colorType,'gray'))
    load('gray_model.mat');
    model.threshold = 30;   
elseif (strcmp(colorType,'purple'))
    load('purple_model.mat');
    model.threshold = 20;     
elseif (strcmp(colorType,'brown'))
    load('brown_model.mat');
    model.threshold = 30;   
elseif (strcmp(colorType,'orange'))
    load('orange_model.mat');
    model.threshold = 50;     
elseif (strcmp(colorType,'white'))
    load('white_model.mat');
    model.threshold = 50;         
elseif (strcmp(colorType,'skin'))
    load('skin_model.mat');
    model.threshold = 50;
else
    disp('Color undefined ...');
end


if (strcmp(colorType,'skin'))
    skin_score = 0; 
else
    skin_score = ComputeProb(data, skin_model);
end

S = ComputeProb(data, model);

% img1 = S > model.threshold;
% img2 = skin_score > 0.1;
% idx = find(img2 == 1);
% img1(idx) = 0;
%detectedImg = reshape(255*img1, [size(Img,1) size(Img,2)]); 
detectedImg = reshape(255*(S > model.threshold & S > skin_score), [size(Img,1) size(Img,2)]); 

ShowResults(filename, detectedImg);



function score = ComputeProb(data, model)

k = length(model.weight);

score = zeros(size(data,2), 1);
for i = 1 : k
    detval = sqrt(det(model.Sigma(:,:,i)));
    sigmaInv = inv(model.Sigma(:,:,i));
    diff = data - repmat(model.mu(:,i), [1 size(data, 2)]);
    %% (x-u)' Sigma^-1 (x-u)
    A = diff' * sigmaInv;
    val = sum(A .* diff', 2);
    score = score + model.weight(i) * exp(-0.5 * val) / detval;
end





function ShowResults(filename, detectedImg)

idx = find(detectedImg > 0);

Img = imread(filename);
rData = double(reshape(Img(:,:,1), [size(Img,1)*size(Img,2) 1]));
gData = double(reshape(Img(:,:,2), [size(Img,1)*size(Img,2) 1]));
bData = double(reshape(Img(:,:,3), [size(Img,1)*size(Img,2) 1]));


rData(idx,1)=255;
gData(idx,1)=255;
bData(idx,1)=255;
rData = reshape(rData, [size(Img,1) size(Img,2)]); 
gData = reshape(gData, [size(Img,1) size(Img,2)]); 
bData = reshape(bData, [size(Img,1) size(Img,2)]); 

Img2 = Img;
Img2(:,:,1) = rData;
Img2(:,:,2) = gData;
Img2(:,:,3) = bData;



subplot(2,2,1);
imshow(Img);
title('Original Image');
subplot(2,2,2);
imshow(detectedImg);
title('Labeled Image');
subplot(2,2,3);
imshow(Img2);
title('Overlayed Image');

