# Roteiro de apresentação — até 10 minutos

## 0:00–1:00 — Tema e objetivo

Apresente o tema 33, Academia. O sistema gerencia alunos de musculação e treinamento funcional. Ambos possuem nome, telefone, mensalidade e data de matrícula. Cada modalidade também possui dados próprios.

Informe o uso de IA conforme a [declaração](uso-de-ia.md): apoio na elaboração e revisão do projeto, incluindo código, testes e documentação.

## 1:00–3:00 — Classes e programação orientada a objetos

Abra o [diagrama UML](diagrama_classes.pdf) e mostre:

1. `Aluno` é a superclasse abstrata, com quatro atributos privados.
2. `Data` é um atributo de `Aluno`: isso representa composição.
3. `AlunoMusculacao` e `AlunoFuncional` herdam de `Aluno` e possuem atributos privados próprios.
4. As subclasses sobrescrevem métodos, como `exibir` e `serializar`.
5. `GerenciadorAcademia` mantém um `vector<unique_ptr<Aluno>>`. Assim, o mesmo vetor armazena alunos das duas subclasses e chama os métodos corretos por polimorfismo.

Explique que os arquivos `.h` contêm as classes e que a `main.cpp` cuida do menu, delegando as operações ao gerenciador.

## 3:00–6:30 — Demonstração do CRUD

Compile com `make`. Use um arquivo exclusivo para a demonstração, para não alterar cadastros pessoais:

```sh
./academia demonstracao.txt
```

Se o arquivo já existir, seus dados serão carregados. Use outro nome para começar com uma lista vazia.

1. Cadastre uma aluna de musculação: `Ana Clara`, telefone `11987654321`, mensalidade `120,00`, objetivo `Fortalecimento` e instrutor `Carlos Silva`.
2. Cadastre uma aluna de treinamento funcional: `Ana Paula`, telefone `11912345678`, mensalidade `150,00`, 3 aulas semanais e nível `Iniciante`.
3. Liste todos e mostre os dados diferentes de cada modalidade.
4. Pesquise `Ana` na opção 3. Mostre os dois resultados com seus índices e escolha um deles.
5. Altere uma mensalidade na opção 4. Pressione Enter nos campos que deseja manter.
6. Mostre a confirmação da remoção na opção 5; você pode cancelar para preservar os dois cadastros.

Abra o método `pesquisar` e mostre que as opções exibir, alterar e remover reutilizam o mesmo retorno de índice.

## 6:30–7:30 — Relatório

Abra a opção 6. Mostre a quantidade de alunos, a soma e a média das mensalidades e o resumo por modalidade. Antes de qualquer alteração no exemplo acima, são dois alunos e R$ 270,00 em mensalidades previstas.

Explique a diferença em relação à listagem: o relatório agrega os dados. Ele apresenta previsão baseada nos cadastros e não comprova recebimento de pagamentos.

## 7:30–8:30 — Arquivo único e persistência

Saia pela opção 7. Abra `demonstracao.txt`: os registros de ambas as subclasses estão no mesmo arquivo e cada linha identifica a modalidade. Execute o programa novamente com o mesmo arquivo e mostre os alunos carregados.

Explique que o vetor recebe todas as operações durante a execução. O arquivo participa apenas da carga e da gravação. A gravação usa um arquivo temporário antes da substituição do arquivo definitivo.

## 8:30–9:30 — Verificação

Execute `make test` e explique que os testes são escritos em C++. Mostre um caso de validação no programa, como uma mensalidade negativa ou uma quantidade de aulas fora de 1 a 7.

## 9:30–10:00 — Perguntas

Reserve o tempo restante para responder sobre herança, composição, polimorfismo e o caminho entre o menu, o vetor e o arquivo.
