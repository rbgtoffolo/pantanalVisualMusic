(
s = Server.default;
s.options.memSize = 2.pow(20);
//s.reboot;

s.waitForBoot({


	// Funções OSC disparado pelo Piezo e que controla os sons da ponte.
	~ofcolor = OSCFunc({ | msg |

		postf("color: % \n", msg);

	}, '/color');

	// OSC que recebe info do PROCESSING para disparar o som do video principal.

	OSCdef.new(\mainVideo, {
		| msg |

		"start".postln;
	}, '/start');

	// OSC que recebe info do PROCESSING para disparar os sons relacionados aos videos sorteados
});

);


OSCdef.freeAll