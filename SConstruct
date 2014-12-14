env = Environment()
env.Append(
  CXXFLAGS=['/DEBUG'])


Program(

    'bin/sample_player',

    [
        'src/clip.c',
        'src/debug.c',
        'src/jack.c',
        'src/main.c',
        'src/osc.c',
    ],

    LIBS=[
        'jack',
        'lo',
        'm',
        'pthread',
        'sndfile',
    ]

)


Program(

    'bin/jackplay',

    'misc/jackplay.c',

    LIBS=[
        'm',
        'pthread',
        'jack',
        'sndfile',
    ]

)
