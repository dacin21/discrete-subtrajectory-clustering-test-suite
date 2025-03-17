from script.cli import build_argument_parser


def main():
    parser = build_argument_parser()
    args = parser.parse_args()
    args.fun(args)


if __name__ == '__main__':
    main()
