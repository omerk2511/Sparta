<template>
    <v-main>
        <v-container class="py-8 px-6" fluid>
            <v-row>
                <v-col cols="12">
                    <v-card v-if="initialized">
                        <v-card-text>
                            <div>{{ threat.date }}</div>
                            <p class="display-1 text--primary mb-1">
                                {{ threat.name }}
                            </p>
                            <p>{{ threat.hash }}</p>
                            <div class="text--primary">
                                {{ threat.description }}
                            </div>
                        </v-card-text>
                        <v-card-actions>
                            <v-btn text color="primary" :to="threat.type.path">
                                Learn More
                            </v-btn>

                            <v-btn text color="primary" :href="`http://localhost:3000/api/dumps/${threat.hash}.bin`" :download="`${threat.hash}.bin`">
                                Memory Dump
                            </v-btn>

                            <v-btn text color="primary" target="_blank" :href="`https://www.virustotal.com/gui/file/${threat.hash}/detection`">
                                View On VirusTotal
                            </v-btn>
                        </v-card-actions>
                    </v-card>
                </v-col>
            </v-row>
        </v-container>
    </v-main>
</template>

<script>
import ThreatsService from '@/services/threats';
import { parseISO, format } from 'date-fns';

export default {
    name: 'Threat',

    title: 'Sparta',

    data: () => ({
        threat: {},
        initialized: false
    }),

    mounted() {
        const id = this.$route.params.id;

        ThreatsService.getThreat(id)
            .then(threat => {
                this.threat = {
                    ...threat,
                    date: format(parseISO(threat.date), 'dd/MM/yyyy HH:mm:ss.SSS'),
                };

                this.initialized = true;
            });
    },
};
</script>
