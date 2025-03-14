import argparse

import cv2
import numpy as np

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, help="input image path")
    parser.add_argument("--output", required=False, default='output.h', help="path to output file")

    return parser.parse_args()


def main():
    args = parse_args()
    image = cv2.imread(args.input, cv2.IMREAD_GRAYSCALE)
    if image is None:
        print(
            f"Could not read image {args.input}. "
            f"Please, check that you passed the correct path"
        )
        exit()

    image = cv2.resize(image, (128, 48))
    image[image > 0] = 1
    image = 1 - image
    image = list(np.ravel(image))
    image_text = ''
    for i, pixel_data in enumerate(image):
        image_text += str(pixel_data)
        if i != len(image) - 1:
            image_text += ', '
    text = f"#ifndef _OUTPUT_\n#define _OUTPUT_\nbyte output[128*64] = {{ {image_text} }};\n\n#endif"

    with open(args.output, 'w') as f:
        f.write(text)

if __name__ == "__main__":
    main()
