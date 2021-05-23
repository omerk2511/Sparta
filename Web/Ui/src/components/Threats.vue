<template>
  <v-main>
    <v-container
      class="py-8 px-6"
      fluid
    >
      <v-row>
        <v-col
          v-for="[date, threats] in cards"
          :key="date"
          cols="12"
        >
          <v-card>
            <v-subheader>{{ date }}</v-subheader>

            <v-list two-line>
              <template v-for="threat in threats">
                <v-list-item
                  :key="threat.id"
                  link
                  :to="`/threat/${threat.id}`"
                >
                  <v-list-item-icon>
                    <v-icon color="secondary">mdi-alert</v-icon>
                  </v-list-item-icon>

                  <v-list-item-content>
                    <v-list-item-title>{{ threat.name }} ({{ threat.hash }})</v-list-item-title>

                    <v-list-item-subtitle>
                      {{ threat.type.name }}
                    </v-list-item-subtitle>
                  </v-list-item-content>
                </v-list-item>

                <v-divider
                  v-if="threats[threats.length - 1].id != threat.id"
                  :key="`divider-${threat.id}`"
                  inset
                ></v-divider>
              </template>
            </v-list>
          </v-card>
        </v-col>
      </v-row>
    </v-container>
  </v-main>
</template>

<script>
  import ThreatsService from '@/services/threats';
  import { parseISO, formatISO, formatRelative, compareAsc } from 'date-fns';
  import { enGB } from 'date-fns/locale';
  import lodash from 'lodash';

  export default {
    name: 'Threats',

    title: 'Sparta',

    data: () => ({
      timerId: 0,
      cards: [],
    }),

    mounted() {
      this.refreshThreats();
      this.timerId = setInterval(this.refreshThreats.bind(this), 3000);
    },

    destroyed() {
      clearInterval(this.timerId);
    },

    methods: {
      refreshThreats: function () {
        ThreatsService.getAllThreats()
          .then(threats => {
            const groupedThreats = lodash.groupBy(threats, threat => {
              const date = parseISO(threat.date);
              const day = new Date(date.getFullYear(), date.getMonth(), date.getDate());

              return formatISO(day);
            });

            const sortedThreats = Object.entries(groupedThreats).sort((a, b) => compareAsc(parseISO(b[0]), parseISO(a[0])));

            const formatRelativeLocale = {
              lastWeek: "'Last' eeee",
              yesterday: "'Yesterday'",
              today: "'Today'",
              other: 'dd/MM/yyyy',
            };

            const locale = {
              ...enGB,
              formatRelative: (token) => formatRelativeLocale[token],
            };

            const formattedThreats = sortedThreats.map(([date, threats]) => {
              return [formatRelative(parseISO(date), new Date(), { locale }), threats.sort((a, b) => compareAsc(parseISO(b.date), parseISO(a.date)))];
            });

            this.cards = formattedThreats;
          });
      }
    }
  }
</script>
