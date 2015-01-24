function Train(filename, outfile, colorType, numClusters)

[r g b] = textread(filename,'%d %d %d',-1);


X = [r g b];
X = rgb2hsv(X);
X(:,3) = X(:,3)/255.;

red = [255 133 132;
       217 125 148;
       254 31 32;
       186 50 60;
       215 104 114;
       170 44 41;
       106 24 24;
       253 57 59;
       170 77 77];
   
green = [156 255 136;
         5 125 125;
         5 225 119;
         48 173 35
         125 244 129;
         117 219 27;
         34 238 91;
         17 119 45;
         62 215 21;
         5  60 70;
         10 80 90];
     
blue = [136 171 255;
        16 1 187;
        33 119 170;
        67 34 237;
        119 137 239;
        1 51 240;
        34 17 119;
        102 101 254;
        85 137 239];
    
      
black = [0 0 0;
         6 4 5;
         9 10 12;
         15 14 18;
         16 17 21;
         25 25 25;
         35 35 35;
         45 45 45
         ];

      
purple = [212 122 250;
          116 10 171;
          66 10 100;
          50 5 74;
          189 40 250;
          194 68 250;
          135 10 201;
          174 25 247;
          172 124 138];
          
      
gray = [47 45 55;
        78 71 64;
        62 58 50;
        80 81 68;
        88 82 90;
        115 107 105;
        130 135 115;
        128 128 128;
        107 111 94;
        123 132 113;
        120 113 100;
        148 144 145];
        
orange = [246 126 92;
          234 136 37;
          200 90 56;
          180 77 39;
          166 87 21;
          156 91 35];    
      
      
yellow = [245 245 10;
          240 215 20;
          240 237 69;
          245 185 15;
          245 225 15;
          240 213 20;
          240 190 20;
          192 153 31;
          217 182 66;
          203 157 49;
          208 172 78
          ];
     
brown = [55 32 16;
         72 55 36;
         86 68 46;
         80 43 18;
         100 75 63;
         113 53 43;
         135 65 45;
         155 79 55;
        ];
    
white = [250 250 250;
         220 220 220;
         170 190 182;
         148 187 184;
         164 185 175;
         209 212 229;
         212 217 236;
         205 210 195;
        ];
    
    
if (strcmp(colorType,'red')) color = red; 
elseif (strcmp(colorType,'green')) color = green; 
elseif (strcmp(colorType,'blue')) color = blue;    
elseif (strcmp(colorType,'black')) color = black;      
elseif (strcmp(colorType,'white')) color = white;    
elseif (strcmp(colorType,'purple')) color = purple;    
elseif (strcmp(colorType,'gray')) color = gray;
elseif (strcmp(colorType,'orange')) color = orange;
elseif (strcmp(colorType,'yellow')) color = green;
elseif (strcmp(colorType,'brown')) color = brown;    
else
    disp('ERROR: color undefined');
    exit;
end
    
n = 64;
level = 6;
XX = zeros(n*n*size(color,1), 3);

for i = 1 : size(color,1)
    Im = zeros(n,n,3);
    Im(:,:,1) = color(i,1);
    Im(:,:,2) = color(i,2);
    Im(:,:,3) = color(i,3);
    
    noise_r = randn(n,n);
    noise_g = randn(n,n);
    noise_b = randn(n,n);
    
    Im(:,:,1) = Im(:,:,1) + level*noise_r;
    Im(:,:,2) = Im(:,:,2) + level*noise_g;
    Im(:,:,3) = Im(:,:,3) + level*noise_b;
    
    idx = find(Im > 255);
    Im(idx) = 255;
    
    idx = find(Im < 1);
    Im(idx) = 1;
    
    Im = rgb2hsv(Im);
    Im(:,:,3) = Im(:,:,3)/255.;

    x1 = (i-1)*n*n+1;
    x2 = i*n*n;
    XX(x1:x2,:) = reshape(Im, [n*n 3]);
end      

%X = XX;
X = [X; XX];    


[label, model, llh] = emgm(X', numClusters);
save(outfile,'model');

