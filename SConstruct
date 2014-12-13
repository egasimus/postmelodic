env = Environment()
env.Append(
  CXXFLAGS=['/DEBUG'])


Program(

    'bin/sample_player',

    [
        'src/debug.c',
        'src/jack.c',
        'src/main.c',
        'src/osc.c',
        'src/reader.c',
    ],

    LIBS=[
        'm',
        'pthread',
        'jack',
        'sndfile',
    ]

)


Program(

    'bin/jackplay',

    'src/jackplay.c',

    LIBS=[
        'm',
        'pthread',
        'jack',
        'sndfile',
    ]

)
