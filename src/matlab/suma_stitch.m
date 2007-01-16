function A = suma_stitch(imaname)
figure(1); clf
[e, em, lst] = zglobb({sprintf('%s*', imaname)});
N_im = length(lst)
N_1 = round(sqrt(N_im))
k=1
figure(1); clf
for (i=1:1:N_1),
   for (j=1:1:N_1),
      %k = ((i-1)+(j-1)*N_1)+1
      lst(k).name
      a = imread(lst(k).name); size(a)
      if (k==1), A = zeros(N_1.*size(a,1), N_1.*size(a,2), size(a,3), 'uint8'); end
      istrt = (N_1-i)*size(a,1) + 1;
      istp = istrt + size(a,1) - 1;
      jstrt = (j-1)*size(a,2) + 1;
      jstp = jstrt + size(a,2) - 1;
      A(istrt:istp,jstrt:jstp,:) = a;
      subplot (N_1, N_1, k); image(a); title(lst(k).name); drawnow; 
      k = k +1;
   end
end

figure(2); clf
image(A); axis square;  drawnow
info = imfinfo(lst(1).name);info.Format
imwrite(A, sprintf('%s_stitch.%s', imaname, info.Format), info.Format);
