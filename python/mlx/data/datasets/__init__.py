# Copyright © 2023 Apple Inc.

from .cifar import load_cifar10, load_cifar100
from .image_folder import load_images_from_folder
from .imagenet import load_imagenet, load_imagenet_metadata
from .librispeech import load_librispeech, load_librispeech_tarfile
from .libritts_r import load_libritts_r, load_libritts_r_tarfile
from .mnist import load_fashion_mnist, load_mnist
from .speechcommands import load_speechcommands
from .wikitext import load_wikitext_lines
