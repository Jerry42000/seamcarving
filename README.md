# Seam Carving

Content-Aware Image Resizing (Seam Carving) — C

Modern photo apps and responsive websites often need to change an image’s size without stretching or cropping away important content. Seam carving solves this by finding a low-energy “seam” of pixels (usually along background or texture) and removing it; repeating the process shrinks width/height while preserving salient objects (faces, text, subjects).

This project implements seam carving in C: it computes per-pixel energy from image gradients, uses dynamic programming to find the minimum-energy vertical seam, backtracks the path, and removes it to reduce width by one pixel (repeat N times for larger changes). 

*How it works

-->Compute energy from horizontal/vertical color differences (image gradient).

-->Accumulate the minimum seam cost with DP down the image, pick the lowest-cost end, backtrack to recover the path, remove those pixels, and stitch the image back together; repeat.
